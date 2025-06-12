
#include "asm_passes.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <stdlib.h>
#include <cstdio>

namespace RA
{

    // liveIn and liveOut for each machine blocks
    static Array<Set<MOperand>> live_in;  // new operands in a specific liveness period
    static Array<Set<MOperand>> live_out; // operands used by successor liveness periods

    static Array<Set<MOperand>> use_set; // actually 'use without def' set
    static Array<Set<MOperand>> def_set;

    void analyse_liveness(Func_Asm *func_asm)
    {

        live_in.release();
        live_out.release();
        use_set.release();
        def_set.release();

        live_in.set_len(func_asm->mbs.len);
        live_out.set_len(func_asm->mbs.len);
        use_set.set_len(func_asm->mbs.len);
        def_set.set_len(func_asm->mbs.len);

        // get use and def sets for each machine block
        for (int i = 0; i < func_asm->mbs.len; i++)
        {
            for (auto I = func_asm->mbs[i]->inst; I; I = I->next)
            {
                Array<MOperand> defs = get_defs(I);
                Array<MOperand> uses = get_uses(I, func_asm->has_return_value);

                for (auto use : uses)
                {
                    if (use.tag == VREG && def_set[i].find(use) == def_set[i].end())
                    {
                        use_set[i].insert(use);
                    }
                }

                for (auto def : defs)
                {
                    def_set[i].insert(def);
                }
            }
        }

        // iteratively updating 'live_in' and 'live_out' until stable
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (int i = 0; i < func_asm->mbs.len; i++)
            {
                Set<MOperand> old_in = live_in[i];
                Set<MOperand> old_out = live_out[i];

                live_in[i] = use_set[i];
                for (auto o : live_out[i])
                {
                    if (def_set[i].find(o) == def_set[i].end())
                    {
                        live_in[i].insert(o);
                    }
                }

                live_out[i].clear();
                for (auto succ : func_asm->mbs[i]->succs)
                {
                    for (auto succ_in : live_in[succ->i])
                    {
                        live_out[i].insert(succ_in);
                    }
                }

                if (!changed)
                {
                    if ((live_in[i] != old_in) || (live_out[i] != old_out))
                    {
                        changed = true;
                    }
                }
            }
        }
    }

    int K = 14; // r0~r12, lr
    const int max_push_num = 16;
    Set<MOperand> precolored;
    Set<MOperand> initial;
    Set<MOperand> simplify_worklist;
    Set<MOperand> freeze_worklist;
    Set<MOperand> spill_worklist;
    Set<MOperand> spilled_nodes;
    Set<MOperand> coalesced_nodes;
    Set<MOperand> colored_nodes;
    Set<MOperand> select_set;
    Array<MOperand> select_stack;
    Array<uint8> needs_save;

    // keep track of already spilled nodes
    // avoid spilling repeatly
    Set<MOperand> already_spilled;

    Set<MI_Move *, MI_Move_Pointer_Cmp> coalesced_moves;
    Set<MI_Move *, MI_Move_Pointer_Cmp> constrained_moves;
    Set<MI_Move *, MI_Move_Pointer_Cmp> frozen_moves;
    Set<MI_Move *, MI_Move_Pointer_Cmp> worklist_moves;
    Set<MI_Move *, MI_Move_Pointer_Cmp> active_moves;


    typedef Pair<MOperand, MOperand> Edge;
    Set<Edge> adj_set;
    Map<MOperand, Set<MOperand>> adj_list;
    Map<MOperand, int> degree; // @default 0?

    Map<MOperand, Set<MI_Move *, MI_Move_Pointer_Cmp>> move_list;
    Map<MOperand, MOperand> alias;
    Map<MOperand, int32> color;
    Map<MOperand, uint32> use_def_count;
    Map<MOperand, uint32> loop_depth;
    Map<MOperand, int32> spill_load_offset;
    Map<MOperand, int32> spill_store_offset;

    int32 spilled_stack_size = 0;
    bool done_post_work = false;
    bool need_to_legalize_imm = false;

#define in(it, set) (((set).find(it)) != ((set).end()))

    bool need_alloc(MOperand opr)
    {
        return (opr.tag == REG || opr.tag == VREG);
    }

    void add_edge(MOperand u, MOperand v);
    void build(Func_Asm *func_asm);
    Set<MOperand> adjacent(MOperand n);
    bool move_related(MOperand n);
    void make_worklist();
    void enable_moves(Set<MOperand> nodes);
    void decrement_degree(MOperand m);
    void simplify();
    void coalesce();
    void add_worklist(MOperand u);
    bool ok(MOperand t, MOperand r);
    bool conservative(Set<MOperand> nodes);
    MOperand get_alias(MOperand n);
    void combine(MOperand u, MOperand v);
    void freeze();
    void freeze_moves(MOperand u);
    void assign_colors();
    void select_spill();
    void color_register_all(Func_Asm *func_asm);
    void recompute_offset(Func_Asm *func_asm);
    void save_registers(Func_Asm *func_asm);
    int nearest_imm_ror(int x);
    void allocate_stack(Func_Asm *func_asm);
    void clear_all();
    void clear_before_allocation();

    void add_edge(MOperand u, MOperand v)
    {
        auto p = Edge{u, v};
        if (!in(p, adj_set) && !(u == v))
        {
            adj_set.insert({u, v});
            adj_set.insert({v, u});

            if (u.tag == VREG && (v.tag == REG || v.tag == VREG))
            {
                adj_list[u].insert(v);
                degree[u]++;
            }

            if (v.tag == VREG && (u.tag == REG || u.tag == VREG))
            {
                adj_list[v].insert(u);
                degree[v]++;
            }
        }
    }

    void build(Func_Asm *func_asm)
    {
        int cnt = 0;
        for (auto b : func_asm->mbs)
        {
            int idx = b->i;
            auto live = live_out[b->i];
            for (auto I = func_asm->mbs[b->i]->last_inst; I; I = I->prev)
            {
                auto defs = get_defs(I);
                auto uses = get_uses(I, func_asm->has_return_value);
                if (I->tag == MI_MOVE)
                {
                    if (need_alloc(defs[0]) && uses.size() > 0 && need_alloc(uses[0]))
                    {
                        for (auto use : uses)
                        {
                            if (!need_alloc(use))
                                continue;
                            live.erase(use);
                            move_list[use].insert((MI_Move *)I);
                        }

                        for (auto def : defs)
                        {
                            move_list[def].insert((MI_Move *)I);
                        }

                        worklist_moves.insert((MI_Move *)I);
                    }
                }

                // @Bug, needs to be allocated?
                for (auto def : defs)
                {
                    if (need_alloc(def))
                    {
                        if (b->loop_depth > loop_depth[def])
                        {
                            loop_depth[def] = b->loop_depth;
                        }
                        for (auto l : live)
                            add_edge(l, def);
                        live.insert(def);
                        use_def_count[def]++;
                    }
                }

                // @Optimize
                Set<MOperand> new_live;
                for (auto u : uses)
                {
                    if (need_alloc(u))
                    {
                        use_def_count[u]++;
                        if (b->loop_depth > loop_depth[u])
                        {
                            loop_depth[u] = b->loop_depth;
                        }
                        new_live.insert(u);
                    }
                }
                // @TODO: record the inserted MOperand and insert
                for (auto l : live)
                {
                    // if (need_alloc(def) && l != def) {
                    if (defs.find(l) == -1)
                    {
                        new_live.insert(l);
                    }
                }
                live = new_live;
            }
        }
    }

    // return adjacent set of n
    Set<MOperand> adjacent(MOperand n)
    {
        Set<MOperand> a;
        auto it = adj_list[n].begin();
        while (it != adj_list[n].end())
        {
            if (!in(*it, select_set) && !in(*it, coalesced_nodes))
            {
                a.insert(*it);
            }
            it++;
        }
        return a;
    }

    // return MI_Move that related to n
    Set<MI_Move *> node_moves(MOperand n)
    {
        Set<MI_Move *> s;
        auto it = move_list[n].begin();
        while (it != move_list[n].end())
        {
            if (in(*it, active_moves) || in(*it, worklist_moves))
            {
                s.insert(*it);
            }
            it++;
        }
        return s;
    }

    // is n related to an active MI_Move
    bool move_related(MOperand n)
    {
        return !node_moves(n).empty();
    }

    // divide initial into spill_worklist, freeze_worklist and simplify_worklist
    void make_worklist()
    {
        while (!initial.empty())
        {
            auto n = *(initial.begin());
            initial.erase(n);
            if (degree[n] >= K)
            {
                spill_worklist.insert(n);
            }
            else if (move_related(n))
            {
                freeze_worklist.insert(n);
            }
            else
            {
                simplify_worklist.insert(n);
            }
        }
    }

    // move active_moves of nodes to worklist_moves 
    void enable_moves(Set<MOperand> nodes)
    {
        for (auto n : nodes)
        {
            for (auto m : node_moves(n))
            {
                if (in(m, active_moves))
                {
                    active_moves.erase(m);
                    worklist_moves.insert(m);
                }
            }
        }
    }

    // decrease degree and try to erase m from spill_worklist
    void decrement_degree(MOperand m)
    {
        int d = degree[m]--;
        if (d == K)
        {
            auto adj = adjacent(m);
            adj.insert(m);
            enable_moves(adj);
            spill_worklist.erase(m);
            if (move_related(m))
            {
                freeze_worklist.insert(m);
            }
            else
            {
                simplify_worklist.insert(m);
            }
        }
    }

    // move the front operand from simplify_worklist to select_stack
    void simplify()
    {
        auto n = *(simplify_worklist.begin());
        simplify_worklist.erase(n);
        select_stack.push(n);
        select_set.insert(n);
        for (auto m : adjacent(n))
        {
            decrement_degree(m);
        }
    }

    // delete 'mv dst, src' and replace 'dst' with 'src'
    void coalesce()
    {
        auto m = *(worklist_moves.begin());
        worklist_moves.erase(m);
        auto u = get_alias(m->src);
        auto v = get_alias(m->dst);
        auto e = Edge{u, v};
        if (v.tag == REG)
        { // @What?
            auto t = u;
            u = v;
            v = t;
        }

        // @Performance
        int okok = false;
        for (auto t : adjacent(v))
        {
            if (ok(t, u))
            {
                okok = true;
                break;
            }
        }

        auto join = adjacent(u);
        for (auto n : adjacent(v))
        {
            join.insert(n);
        }

        if (u == v)
        {
            coalesced_moves.insert(m);
            add_worklist(u);
        }
        else if (v.tag == REG || in(e, adj_set))
        {
            constrained_moves.insert(m);
            add_worklist(u);
            add_worklist(v);
        }
        else if ((u.tag == REG && okok) ||
                 (u.tag != REG && conservative(join)))
        {
            coalesced_moves.insert(m);
            combine(u, v);
            add_worklist(u);
        }
        else
        {
            active_moves.insert(m);
        }
    }

    // try to add u from freeze_worklist to simplify_worklist
    void add_worklist(MOperand u)
    {
        if (u.tag != REG && !move_related(u) && degree[u] < K)
        {
            freeze_worklist.erase(u);
            simplify_worklist.insert(u);
        }
    }

    bool ok(MOperand t, MOperand r)
    {
        auto p = Edge{t, r};
        return ((degree[t] < K) || (t.tag == REG) || (in(p, adj_set)));
    }

    // return 'k < K' where k is the number of nodes that 'degree[n] >= K'
    bool conservative(Set<MOperand> nodes)
    {
        int k = 0;
        for (auto n : nodes)
        {
            if (degree[n] >= K)
            {
                k++;
            }
        }
        return k < K;
    }

    MOperand get_alias(MOperand n)
    {
        if (in(n, coalesced_nodes))
        {
            return get_alias(alias[n]);
        }
        else
        {
            return n;
        }
    }

    // use 'u' to replace 'v'
    void combine(MOperand u, MOperand v)
    {
        if (in(v, freeze_worklist))
        {
            freeze_worklist.erase(v);
        }
        else
        {
            spill_worklist.erase(v);
        }
        coalesced_nodes.insert(v);
        alias[v] = u;

        // FIXME
        for (auto n : move_list[v])
        {
            move_list[u].insert(n);
        }

        for (auto t : adjacent(v))
        {
            add_edge(t, u);
            decrement_degree(t);
        }

        if (degree[u] >= K && in(u, freeze_worklist))
        {
            freeze_worklist.erase(u);
            spill_worklist.insert(u);
        }
    }

    // move the front operand of freeze_worklist to simplify_worklist
    void freeze()
    {
        auto u = *(freeze_worklist.begin());
        freeze_worklist.erase(u);
        simplify_worklist.insert(u);
        freeze_moves(u);
    }

    void freeze_moves(MOperand u)
    {
        for (auto m : node_moves(u))
        {
            // @Order?
            auto u = m->src;
            auto v = m->dst;

            if (in(m, active_moves))
            {
                active_moves.erase(m);
            }
            else
            {
                worklist_moves.erase(m);
            }

            frozen_moves.insert(m);
            if (node_moves(v).empty() && degree[v] < K)
            {
                freeze_worklist.erase(v);
                simplify_worklist.insert(v);
            }
        }
    }

    // @Optimization: can we use lr?
    bool is_general_purpose_register(uint8 r)
    {
        return ((r >= r0) && (r <= r12)) || r == lr;
    }

    void assign_colors()
    {
        while (!select_stack.empty())
        {
            auto n = select_stack.back();
            select_stack.pop();
            Array<uint8> ok_colors;

            // assign caller-save first
            ok_colors.push(r0);
            ok_colors.push(r1);
            ok_colors.push(r2);
            ok_colors.push(r3);
            ok_colors.push(r12);

            // callee-save regs
            for (uint8 r = r4; r <= r11; r++)
            {
                ok_colors.push(r);
            }
            ok_colors.push(lr);

            for (auto w : adj_list[n])
            {
                auto a = get_alias(w);
                if (in(a, colored_nodes) || in(a, precolored))
                {
                    ok_colors.remove(color[a]);
                }
            }
            if (ok_colors.empty())
            {
                spilled_nodes.insert(n);
            }
            else
            {
                colored_nodes.insert(n);
                auto c = ok_colors.front();
                color[n] = c;
            }
        }
        for (auto n : coalesced_nodes)
        {
            color[n] = color[get_alias(n)];
        }
    }

    void select_spill()
    {
        // auto m = *(spill_worklist.begin());
        MOperand m = {};
        double min_cost = 1e40;
        bool is_already_spilled = false;
        Set<MOperand> spilled;
        for (auto it = spill_worklist.begin();
             it != spill_worklist.end();
             it++)
        {
            // when regs already spilled, erase from spill_worklist
            if (in(*it, already_spilled)) {
                is_already_spilled = true;
                spilled.insert(*it);
                continue;
            }

            assert(degree[*it] != 0);

            double cost = use_def_count[*it] * pow(10, loop_depth[*it]) / degree[*it];

            if (cost < min_cost)
            {
                m = *it;
                min_cost = cost;
            }
        }

        if (m.tag == ERRORTYPE && is_already_spilled) {
            for (auto it = spilled.begin(); it != spilled.end(); it++) {
                spill_worklist.erase(*it);
                simplify_worklist.insert(*it);
                freeze_moves(*it);
            }
            if (m.tag == ERRORTYPE) return;
        }
        assert(m.tag != ERRORTYPE);

        spill_worklist.erase(m);
        simplify_worklist.insert(m);
        freeze_moves(m);
    }

    void color_register(MOperand &m)
    {
        if (m.tag == VREG)
        {
            assert(color.find(m) != color.end());
            uint8 reg = color[m];
            m.value = reg;
            m.tag = REG;
        }

        if (m.tag == REG && is_callee_save(m.value))
            if (needs_save.find(m.value) == -1)
                needs_save.push(m.value);
    }

    void allocate_register(Func_Asm *func_asm)
    {
        clear_all();

        analyse_liveness(func_asm);

        for (int i = 0; i < REG_COUNT; i++) {
            auto r = make_reg(i);
            precolored.insert(r);
            color[r] = i;
        }

        for (int i = 0; i < func_asm->vreg_count; i++) {
            initial.insert(make_vreg(i));
        }

        build(func_asm);
        make_worklist();
        do {
            if (!simplify_worklist.empty())
                simplify();
            else if (!worklist_moves.empty())
                coalesce();
            else if (!freeze_worklist.empty())
                freeze();
            else if (!spill_worklist.empty())
                select_spill();
        } while (
            !simplify_worklist.empty() ||
            !worklist_moves.empty() ||
            !freeze_worklist.empty() ||
            !spill_worklist.empty());

        assign_colors();  
        if (!spilled_nodes.empty()) {
            // insert stores and restores for each spilled node
            for (MOperand n : spilled_nodes) {
                already_spilled.insert(n);

                printf("spilling nodes: ");
                print_operand(n);
                printf("\n");
                spilled_stack_size += 4;
                bool is_defined = false;

                for (auto bb : func_asm->mbs) {
                    bool marked = false;
                    for (auto I = bb->inst; I; I = I->next) {
                        auto defs = get_defs(I);
                        auto uses = get_uses(I, func_asm->has_return_value);

                        bool inserted_store = false;
                        bool inserted_use = false;
                        // insert store after def
                        if (defs.find(n) != -1) {
                            // avoid load -> store
                            // if(I->tag == MI_LOAD && spill_store_offset.count(n)) {
                            //     auto load = (MI_Load *)I;
                            //     if(load->mem_tag == MEM_LOAD_SPILL) {
                            //         I->mark();
                            //         marked = true;
                            //         continue;
                            //     }
                            // }

                            auto str = new MI_Store;
                            // str->reg = make_vreg(func_asm->vreg_count++);
                            // @IMPORTANT: do not allocate new reg
                            str->reg = n;
                                
                            str->base = make_reg(sp);
                            str->offset = make_imm(-(spilled_stack_size));
                            str->mem_tag = MEM_SAVE_SPILL;
                            // already_spilled.insert(str->reg);

                            // replace_defs(I, &n, &str->reg, func_asm);
                            insert((MI *)str, I->next); // @Last?
                            I = I->next;                // jump past newly inserted str
                            inserted_store = true;
                            is_defined = true;
                            spill_store_offset[n] = -(spilled_stack_size);
                        }

                        // insert load before use
                        if (uses.find(n) != -1) {
                            // avoid load -> store
                            if(I->tag == MI_STORE && spill_load_offset.count(n)) {
                                auto store = (MI_Store *)I;
                                if(store->mem_tag == MEM_SAVE_SPILL) {
                                    I->mark();
                                    marked = true;
                                    continue;
                                }
                            }

                            auto ldr = new MI_Load;
                            ldr->mem_tag = MEM_LOAD_SPILL;
                            ldr->reg = make_vreg(func_asm->vreg_count++);
                            // @IMPORTANT: do not allocate new reg
                            // ldr->reg = n;
            
                            ldr->base = make_reg(sp);
                            ldr->offset = make_imm(-(spilled_stack_size));
                            already_spilled.insert(ldr->reg);
                            replace_uses(I, &n, &ldr->reg, func_asm);
                            // @TODO: get uses based on arg count
                            insert((MI *)ldr, I);
                            inserted_use = true;
                            spill_load_offset[ldr->reg] = -(spilled_stack_size);
                        }
                        assert(!(inserted_use && inserted_store));
                    }
                    if(marked)
                        bb->erase_marked_values();
                }
                if(!is_defined) {
                    fprintf(stderr, "error: vr%d is not defined\n", n.value);
                    fflush(stderr);
                }
                assert(is_defined);
            }
            func_asm->stack_size += spilled_stack_size;
            func_asm->reg_spill_size += spilled_stack_size;

            printf(">>> spilled stack size: %d\n", spilled_stack_size);
            printf(">>> after spilling: \n");
            printf("\n");
        }

    }

    void color_register_all(Func_Asm *func_asm) {
        // done, replace all vreg with actual regs
        for (auto b : func_asm->mbs) {
            for (auto I = func_asm->mbs[b->i]->inst; I; I = I->next) {
                switch (I->tag) {

                case MI_CLZ: {
                    color_register(((MI_Clz *)I)->dst);
                    color_register(((MI_Clz *)I)->operand);
                }
                break;

                case MI_MOVE: {
                    color_register(((MI_Move *)I)->dst);
                    color_register(((MI_Move *)I)->src);
                }
                break;

                case MI_BINARY: {
                    color_register(((MI_Binary *)I)->dst);
                    color_register(((MI_Binary *)I)->lhs);
                    color_register(((MI_Binary *)I)->rhs);
                }
                break;

                case MI_COMPLEXMUL: {
                    color_register(((MI_ComplexMul *)I)->dst);
                    color_register(((MI_ComplexMul *)I)->lhs);
                    color_register(((MI_ComplexMul *)I)->rhs);
                    color_register(((MI_ComplexMul *)I)->extra);
                }
                break;

                case MI_COMPARE: {
                    color_register(((MI_Compare *)I)->lhs);
                    color_register(((MI_Compare *)I)->rhs);
                }
                break;

                case MI_LOAD:
                case MI_STORE: {
                    color_register(((MI_Load *)I)->reg);
                    color_register(((MI_Load *)I)->base);
                    color_register(((MI_Load *)I)->offset);
                }
                break;

                case MI_RETURN: {
                    auto return_addr = make_reg(lr);
                    color_register(return_addr);
                } break;

                case MI_VMOVE: {
                    auto vmv = ((MI_VMove *)I);
                    if (vmv->dst.tag == REG || vmv->dst.tag == VREG)
                    {
                        color_register(vmv->dst);
                    }
                    if (vmv->src.tag == REG || vmv->src.tag == VREG)
                    {
                        color_register(vmv->src);
                    }
                }
                break;

                case MI_VLOAD:
                case MI_VSTORE: {
                    auto vmv = ((MI_VLoad *)I);
                    if (vmv->reg.tag == REG || vmv->reg.tag == VREG)
                    {
                        color_register(vmv->reg);
                    }
                    if (vmv->base.tag == REG || vmv->base.tag == VREG)
                    {
                        color_register(vmv->base);
                    }
                    if (vmv->offset.tag == REG || vmv->offset.tag == VREG)
                    {
                        color_register(vmv->offset);
                    }
                }
                break;

                case MI_VBINARY:
                case MI_VCOMPARE:
                case MI_VPUSH:
                case MI_VPOP:
                case MI_VCVT:
                case MI_FUNC_CALL:
                case MI_PUSH: // we don't push/pop virtual regs, for now
                case MI_POP:
                case MI_BRANCH:
                    break;

                default: {
                    fprintf(stderr, "MI_tag: %d\n", I->tag);
                    assert(false && "unknown MI_tag in register_allocation");
                }
                }
            }
        }

    }

    // recompute all offset related to sp
    void recompute_offset(Func_Asm *func_asm) {
        // convert fp to sp
        // now func_asm->stack_size = size of (local arrays + spilled vars + max calling arguments);
        // not including the callee-save registers' space(push/pop will automatically modify sp)
        // allocate stack space for argument passing
        int stack_arg_size = 0;
        for (auto mb : func_asm->mbs) {
            for (auto I = mb->inst; I; I = I->next) {
                if (I->tag == MI_FUNC_CALL) {
                    auto call = (MI_Func_Call *)I;
                    if (call->arg_stack_size > stack_arg_size) {
                        stack_arg_size = call->arg_stack_size;
                    }
                }
            }
        }
        if(stack_arg_size > 0) 
            func_asm->stack_size += stack_arg_size;

        // align to 8B before stack allocation
        int save_stack_size = (func_asm->reg_needs_save + func_asm->sreg_needs_save) * 4;
        if ((func_asm->stack_size + save_stack_size) % 8 != 0) {
            func_asm->stack_size += 8 - ((func_asm->stack_size + save_stack_size) % 8);
        }

        // fixup sp when calculating base address for local arrays
        for (auto base : func_asm->local_array_bases) {
            assert(base->tag == MI_BINARY);
            auto sub = (MI_Binary *)base;
            assert(sub->op == BINARY_SUBTRACT);
            assert(sub->lhs == make_reg(sp));
            assert(sub->rhs.tag == IMM && sub->rhs.value > 0);

            int offset_relative_to_sp = func_asm->stack_size - sub->rhs.value;
            // @TODO replace with a mov directly
            // if offset relative to sp is 0
            // assert(offset_relative_to_sp % 8 == 0);

            int nearest = nearest_imm_ror(offset_relative_to_sp);
            auto last_dst = make_reg(sp);
            do {
                auto array_offset = new MI_Binary(BINARY_ADD, sub->dst, last_dst, make_imm(nearest));
                insert(array_offset, sub);
                last_dst = sub->dst;
                offset_relative_to_sp -= nearest;
                nearest = nearest_imm_ror(offset_relative_to_sp);
            } while(nearest);

            // sub->op = BINARY_ADD;
            // sub->lhs = make_reg(sp);
            // sub->rhs = make_imm(offset_relative_to_sp);
            sub->erase_from_parent();
        }

        int all_spill_stack_size = func_asm->reg_spill_size + func_asm->sreg_spill_size;
        for (auto bb : func_asm->mbs)
        {
            for (auto I = bb->inst; I; I = I->next)
            {
                // int save_stack_size = 0;
                if (I->tag == MI_LOAD)
                {
                    auto ldr = (MI_Load *)I;
                    if (ldr->mem_tag == MEM_LOAD_SPILL)
                    {
                        int32 offset_value = ((ldr->offset.tag == ERRORTYPE) ? 0 : ldr->offset.value);
                        int32 offset_relative_to_sp = all_spill_stack_size + stack_arg_size + offset_value;
                        ldr->base.value = sp;
                        ldr->offset = make_imm(offset_relative_to_sp);
                    }
                    else if (ldr->mem_tag == MEM_LOAD_ARG)
                    {
                        int32 offset_value = ((ldr->offset.tag == ERRORTYPE) ? 0 : ldr->offset.value);
                        int32 offset_relative_to_sp = func_asm->stack_size + offset_value + save_stack_size;
                        ldr->base.value = sp;
                        ldr->offset = make_imm(offset_relative_to_sp);
                    }
                }
                else if (I->tag == MI_STORE)
                {
                    auto str = (MI_Store *)I;
                    if (str->mem_tag == MEM_SAVE_SPILL)
                    {
                        int32 offset_value = ((str->offset.tag == ERRORTYPE) ? 0 : str->offset.value);
                        int32 offset_relative_to_sp = all_spill_stack_size + stack_arg_size + offset_value;
                        str->base.value = sp;
                        str->offset = make_imm(offset_relative_to_sp);
                    }
                }
                else if (I->tag == MI_VLOAD)
                {
                    auto ldr = (MI_VLoad *)I;
                    if (ldr->mem_tag == MEM_LOAD_SPILL)
                    {
                        int32 offset_value = ((ldr->offset.tag == ERRORTYPE) ? 0 : ldr->offset.value);
                        int32 offset_relative_to_sp = all_spill_stack_size - func_asm->reg_spill_size + stack_arg_size + offset_value;
                        ldr->base.value = sp;
                        ldr->offset = make_imm(offset_relative_to_sp);
                    }
                    else if (ldr->mem_tag == MEM_LOAD_ARG)
                    {
                        int32 offset_value = ((ldr->offset.tag == ERRORTYPE) ? 0 : ldr->offset.value);
                        int32 offset_relative_to_sp = func_asm->stack_size + offset_value + save_stack_size;
                        ldr->base.value = sp;
                        ldr->offset = make_imm(offset_relative_to_sp);
                    }
                }
                else if (I->tag == MI_VSTORE)
                {
                    auto str = (MI_VStore *)I;
                    if (str->mem_tag == MEM_SAVE_SPILL)
                    {
                        int32 offset_value = ((str->offset.tag == ERRORTYPE) ? 0 : str->offset.value);
                        int32 offset_relative_to_sp = all_spill_stack_size - func_asm->reg_spill_size + stack_arg_size + offset_value;
                        str->base.value = sp;
                        str->offset = make_imm(offset_relative_to_sp);
                    }
                }
            }
        }
    }

    // save callee-save registers(push and pop)
    void save_registers(Func_Asm *func_asm) {
        func_asm->reg_needs_save = needs_save.size();
        if (!needs_save.empty()) {
            // insert push in the start block
            std::sort(needs_save.begin(), needs_save.end());
            auto store = new MI_Push;
            for (auto r : needs_save) {
                store->operands.push(make_reg(r + r0));
            }
            insert((MI *)store, func_asm->mbs[0]->inst);

            // restore registers at exit

            for (auto bb : func_asm->mbs) {
                if (bb->last_inst && bb->last_inst->tag == MI_RETURN) {
                    auto restore = new MI_Pop;
                    for (auto r : needs_save) {
                        restore->operands.push(make_reg(r + r0));
                    }
                    insert((MI *)restore, bb->last_inst);
                }
            }
        }
    }

    int nearest_imm_ror(int x) {
        if (can_be_imm_ror(x) || x < 256)
        return x;

        int i = 31;
        while ( !( ( (1 << i) & x) >> i ) ) 
            i--;

        int result = 0xff;
        if (i % 2) 
            result = result << (i-7);
        else 
            result = result << (i-6);
        return x & result;
    }

    // allocate stack based on spilled size, callee-save reg size and call size
    void allocate_stack(Func_Asm *func_asm) {
        // insert add/sub to set up or destroy the stack
        // if stack is too large, allocate it for several times
        if (func_asm->stack_size != 0)
        {
            vector<MI_Binary *> alloc_stack_list;
            vector<MI_Binary *> destroy_stack_list;

            int cur_stack_size = func_asm->stack_size;
            int nearest = nearest_imm_ror(cur_stack_size);
            while(nearest) {
                alloc_stack_list.push_back(new MI_Binary(BINARY_SUBTRACT, make_reg(sp), make_reg(sp), make_imm(nearest)));
                destroy_stack_list.push_back(new MI_Binary(BINARY_ADD, make_reg(sp), make_reg(sp), make_imm(nearest)));
                cur_stack_size -= nearest;
                nearest = nearest_imm_ror(cur_stack_size);
            }
            assert(alloc_stack_list.size());

            auto start_inst = func_asm->mbs[0]->inst;
            // allocate stack after saving callee-save regs
            while (start_inst != NULL && (start_inst->tag == MI_PUSH || start_inst->tag == MI_VPUSH)) {
                start_inst = start_inst->next;
            }
            assert(start_inst != NULL);
            for(int i = alloc_stack_list.size() - 1; i >= 0; i --) {
                insert(alloc_stack_list[i], start_inst);
            }
            
            for (auto bb : func_asm->mbs)
            {
                if (bb->last_inst && bb->last_inst->tag == MI_RETURN)
                {
                    auto last_inst = bb->last_inst;

                    // destroy stack before restoring callee-save regs
                    while (last_inst != NULL && last_inst->prev != NULL 
                        && (last_inst->prev->tag == MI_POP || last_inst->prev->tag == MI_VPOP)) {
                        last_inst = last_inst->prev;
                    }
                    // insert((MI *)destroy_stack, last_inst);
                    for(int i = 0; i < destroy_stack_list.size(); i ++) {
                        insert(destroy_stack_list[i], last_inst);
                    }
                }
            }
        }
    }

    void clear_all() {
        precolored.clear();
        initial.clear();
        simplify_worklist.clear();
        freeze_worklist.clear();
        spill_worklist.clear();
        spilled_nodes.clear();
        coalesced_nodes.clear();
        colored_nodes.clear();
        coalesced_moves.clear();
        constrained_moves.clear();
        frozen_moves.clear();
        select_set.clear();
        worklist_moves.clear();
        active_moves.clear();
        adj_set.clear();
        adj_list.clear();
        degree.clear();
        move_list.clear();
        alias.clear();
        use_def_count.clear();
        loop_depth.clear();

        select_stack.len = 0;
    }

    void clear_before_allocation() {
        color.clear();
        needs_save.len = 0;
    }

    void allocate_single_function(Func_Asm *func_asm, bool opt)
    {
        spilled_stack_size = 0;
        done_post_work = false;
        already_spilled.clear();
        spill_load_offset.clear();
        spill_store_offset.clear();
        do {
            allocate_register(func_asm);
        } while(!spilled_nodes.empty());
    }
}

void register_allocation(Program_Asm *program_asm, bool opt)
{
    for (auto f : program_asm->functions)
    {
        RA::clear_before_allocation();
        RA::allocate_single_function(f, opt);

        RA::color_register_all(f);

        RA::save_registers(f);
        RA::recompute_offset(f);
        RA::allocate_stack(f);
    }
}

