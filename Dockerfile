# v5
FROM ubuntu:20.04

ENV TZ=Asia/Shanghai
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    libjsoncpp-dev \
    ragel \
    libmysqlclient-dev \
    libsqlite3-dev \
    libevent-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libssl-dev \
    libtinyxml2-dev \
    libjemalloc-dev \
    libtinyxml2-dev \
    libjemalloc-dev \
    openjdk-11-jre-headless \
    maven \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /usr/src/workspace

# 安装 yaml-cpp 
RUN git clone --depth 1 --branch yaml-cpp-0.7.0 https://github.com/jbeder/yaml-cpp.git && \
    cd yaml-cpp && \
    rm -rf build && mkdir build && cd build && \
    cmake -DYAML_BUILD_SHARED_LIBS=ON .. && \
    make -j$(nproc) && \
    make install && \
    cd ../.. && \
    rm -rf yaml-cpp


# 安装 hiredis-vip
RUN git clone --depth 1 https://github.com/vipshop/hiredis-vip.git && \
    cd hiredis-vip && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf hiredis-vip

RUN git clone --depth 1 https://github.com/hvoigt/cppunit.git && \
    cd cppunit && \
    ./autogen.sh && \
    ./configure && \
    make -j$(nproc) && \
    make install && \
    cp cppunit.m4 /usr/share/aclocal/ && \
    cd .. && \
    rm -rf cppunit

# 安装 zookeeper-client-c 
RUN git clone --depth 1 --branch release-3.7.0 https://github.com/apache/zookeeper.git && \
    cd zookeeper/zookeeper-jute && \
    # mvn --batch-mode compile && \
    mvn compile -DskipTests && \
    cd ../zookeeper-client/zookeeper-client-c && \
    autoreconf -if && \
    ./configure && \
    make -j$(nproc) && \
    make install && \
    cd ../../.. && \
    rm -rf zookeeper


# 安装系统工具和开发环境
RUN apt-get update && \
    apt-get install -y \
    # neovim \
    vim \
    tmux \
    sudo \
    zsh \
    openssh-server \
    curl \
    wget \
    fonts-powerline \
    net-tools \
    man \
    gdb \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /var/run/sshd 

RUN useradd -m -s /bin/zsh guo && \
    echo 'guo:gchh' | chpasswd && \
    usermod -aG sudo guo && \ 
    echo "guo ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY --chown=guo:guo ./sylar /home/guo/workspace/sylar

WORKDIR /home/guo/workspace
RUN cd ./sylar && \
    rm -rf build && \
    mkdir build && \
    cmake -Bbuild .

# 修改 SSH 配置：允许密码登录，只允许指定用户 guo
RUN sed -i 's/^#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config  && \
    sed -i 's/^#PermitRootLogin .*/PermitRootLogin no/' /etc/ssh/sshd_config && \
    echo "AllowUsers guo" >> /etc/ssh/sshd_config

# 复制公钥文件
# cp ~/.ssh/id_rsa.pub authorized_keys
COPY authorized_keys /home/guo/.ssh/authorized_keys

RUN chown -R guo:guo /home/guo

# 暴露 SSH 端口
EXPOSE 22

CMD ["/usr/sbin/sshd", "-D"]

# # 运行容器
# docker run -it -p:2222:22 -d --name sylar --hostname sylar sylar:v12

## 记得拷贝公钥到 github