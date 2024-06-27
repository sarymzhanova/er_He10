ARG ER=dev
ARG ACCDAQ=master
ARG DEPENDENCIES=ghcr.io/flnr-jinr/fs_oct17p4:latest

FROM ${DEPENDENCIES}

RUN apt-get install nano

WORKDIR /opt

RUN useradd -m -d /home/jovyan jovyan
RUN chown jovyan:jovyan /opt
USER jovyan

SHELL ["/bin/bash", "--login", "-c"]

RUN git clone https://github.com/flnr-jinr/ACCULINNA_go4_user_library accdaq &&\
	cd accdaq &&\
	git checkout ${ACCDAQ} &&\
	mkdir build && cd build &&\
	source /opt/FairSoft/bin/thisroot.sh &&\
	source /opt/go4/go4login &&\
	cmake ../ -DCMAKE_INSTALL_PREFIX=/opt/accdaq/install &&\
	make install -j4

RUN cd /opt && git clone https://github.com/flnr-jinr/er &&\
	cd er &&\
	git checkout ${ER} &&\
	export SIMPATH=/opt/FairSoft/ &&\
	export FAIRROOTPATH=/opt/FairRoot/ &&\
	mkdir build &&\
	cd build &&\
	cmake ../ -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DACCULINNA_GO4=/opt/accdaq/install &&\
	make -j4
#RUN echo "[init]" >> /opt/er/.git/config
#RUN echo "	defaultBranch = ${ER}"

COPY entrypoint.sh /etc/entrypoint.sh
RUN echo ". /etc/entrypoint.sh" >> /home/jovyan/.bashrc
ENTRYPOINT ["/etc/entrypoint.sh"]
