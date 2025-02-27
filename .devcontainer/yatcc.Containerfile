# syntax=docker/dockerfile:1
FROM ubuntu:24.04 AS base

# 安装依赖
RUN apt-get update -y
RUN apt-get install -y --no-install-recommends \
        ca-certificates tini build-essential git gdb
RUN apt-get install -y --no-install-recommends \
        cmake ninja-build default-jdk bison flex lld
RUN apt-get install -y --no-install-recommends \
        python3 python3-yaml
RUN apt-get install -y --no-install-recommends \
        qemu-user-static
RUN apt-get autoremove -y && \
    apt-get clean -y

# 使用 tini 作为开发容器的 PID 1，和 docker run --init 是同样的效果
ENTRYPOINT ["/bin/tini", "--"]
CMD ["sleep", "infinity"]

# 完全版和基础版的区别在于预构建的文件，这会显著影响镜像大小
FROM base AS full

# 复制预构建的文件
COPY antlr /dat/antlr
COPY llvm /dat/llvm

# 设置环境变量
ENV YatCC_ANTLR_DIR=/dat/antlr \
    YatCC_LLVM_DIR=/dat/llvm \
    PATH=/dat/llvm/install/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
