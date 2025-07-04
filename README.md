# ER
ExpertRoot(ER) is a [FairRoot](https://github.com/FairRootGroup/FairRoot)-based framework dedicated
to the simulation, reconstruction, data acquisition, and analysis of the nuclear physics experiments.
Primarily developed for the needs of experiments on fragment separator 
[ACCULLINA-2](http://aculina.jinr.ru/a-2.html) at Fleurov Nuclear Research in JINR and experiment
[EXPERT](http://aculina.jinr.ru/expert.html) on fragment separator SUPER-FRS at FAIR in GSI.

## Library Stack

Simulation, reconstruction and analysis in the ER is based on
* [FairSoft](https://github.com/FairRootGroup/FairSoft/tree/dev) - includes widespread Geant4, ROOT,
   VMC and others utilities.
* [FairRoot](https://github.com/FairRootGroup/FairRoot) - framework which provides the base classes
  for IO-management, base classes event-based _tasks_ for the simulation and reconstruction.

Experimental data processing
* [Go4](https://www.gsi.de/en/work/research/experiment_electronics/data_processing/data_analysis/the_go4_home_page.htm) -
  framework for on-line and off-line processing of experimental data.
* [AccDAQ](https://github.com/FLNR-JINR/ACCULINNA_go4_user_library) - library for unpacking and 
  transformation of the raw experimental data in _lmd_ file format produced by Go4 to _root_ format.
  Output files are applicable for the analysis in the ExpertRoot. 

ER extends FairRoot with experiment-specific tasks and adds several features which enables to use it
for different studies.

## Key features:

* User-defined interactions: reactions and decays.
* It is possible to take into account energy losses of projectiles in the target and the beam 
  detector components such as ToF scintillators and MWPC stations.
* Simulation and analysis of cocktail and defocusing beam.
* Accounting for energy losses in the dead layers of the detector during reconstruction of ejectiles
  energies by experimental data.

## Run using Docker

Due to high dependencies on the external packages of various versions, we strongly recommend to use
the docker-image [docker](https://www.docker.com) to deploy ER installation.

0. Install docker engine on your system: https://docs.docker.com/engine/install/ubuntu/
1. Follow docker post-installation steps: https://docs.docker.com/engine/install/linux-postinstall/
2. Clone ER repository:

```
git clone https://github.com/sarymzhanova/er_He10 .
git checkout ND_2n_track
```

3. Build docker image with _ER_:

```
docker build -t er_nd_2n_track .
```

4. Run _er_ container, compile updated sources:


```
#run docker container
docker run \
    --entrypoint /bin/bash \
    --net=host \
    -v $(pwd):/opt/er \
    -v $(pwd)/macro:/opt/run \
    -v /tmp/.X11-unix:/tmp/.X11-unix  \
    -v $HOME/.Xauthority:/home/jovyan/.Xauthority:rw \
    -w /opt/run \
    -e DISPLAY=$DISPLAY \
    -it er_nd_2n_track:latest

#when run for the first time the ER classes have to be compiled
cd /opt/er
mkdir build
cd build
export SIMPATH=/opt/FairSoft/
export FAIRROOTPATH=/opt/FairRoot/
cmake ../ -DACCULINNA_GO4=/opt/accdaq/install/
make -j4
source ./config.sh

```
here -v is used to map your host working directory '$(pwd)/macro' and working 
directory  '/opt/run' inside container; -w /opt/run - set working directory for interactive session;
-e DISPLAY=$DISPLAY is needed to forward gui from container to host machine; -it - to set interactive session.

In case of adding new classes supplement CMakeLists.txt in the folder with you class files. 
Modify following blocks of code -- INCLUDE_DIRECTORIES and SRCS.

To compile your changes run docker container and execute:

```
cd /opt/er/build
cmake ../ -DACCULINNA_GO4=/opt/accdaq/install/
make -j4

```

5. Run simulation

Here is an example of a Monte-Carlo simulation of 3H(8He,p)10He reaction and following decay of 10He.
```
#run docker container from the root directory of the project
docker run \
    --entrypoint /bin/bash \
    --net=host \
    -v $(pwd):/opt/er \
    -v $(pwd)/macro:/opt/run \
    -v /tmp/.X11-unix:/tmp/.X11-unix  \
    -v $HOME/.Xauthority:/home/jovyan/.Xauthority:rw \
    -w /opt/run \
    -e DISPLAY=$DISPLAY \
    -it er_nd_2n_track:latest

#prepare files with target and wall of neutron detectors
cd /opt/er/macro/geo
root -l create_target_10he_3h_steel_geo.C
root -l create_ND_geo_exp1904_10he_8m.C

#run your simulation with 1000 events
cd /opt/er/macro/He10/sim_nd
root -l 'sim_digi.C(1000)'

cd reco/
root -l reco_10he_exp.C
root -l AfterReco.C
root -l DrawPID.C
```
This is the result of an expected reconstruction:

![alt text](https://github.com/sarymzhanova/er_He10/blob/ND_2n_track/macro/He10/sim_nd/reco/pid.png)



6. Run reconstruction and analysis

This is an example of processing of raw data (unpacking, digitization and reconstruction) 
obtained in 2H(8He,3He)7H reaction:

```
#run docker container
docker run \
    --entrypoint /bin/bash \
    --net=host \
    -v $(pwd):/opt/er \
    -v $(pwd)/macro:/opt/run \
    -v /tmp/.X11-unix:/tmp/.X11-unix  \
    -v $HOME/.Xauthority:/home/jovyan/.Xauthority:rw \
    -w /opt/run \
    -e DISPLAY=$DISPLAY \
    -it er_nd_2n_track:latest

cd /opt/run/EXP1904_H7/input

#download data file. NB! link is temporary, serves only as an example
wget https://filebin.net/hetwtgks8xort1oh/h7_ct_18_0001.lmd

bash prepare_unpack.sh -f h7_ct_18_0001.lmd

cd /opt/accdaq
bash run.sh
cd /opt/run/EXP1904_H7
bash run.sh -f input/h7_ct_18_0001.lmd.root
root -l DrawPID.C
```
This is the result of an expected reconstruction of the experimental data:

![alt text](https://github.com/sarymzhanova/er_He10/blob/ND_2n_track/macro/EXP1904_H7/pid.png)


## Step by Step installation

In case Docker-image installation is not preferable or impossible in some reasons one can install 
all dependencies using the following instructions.

### 1. Install [FairSoft](https://github.com/FairRootGroup/FairSoft/tree/dev)

```
mkdir ~/fair_install
cd ~/fair_install
git clone https://github.com/FairRootGroup/FairSoft.git
cd FairSoft
git checkout BRANCHE_NAME
./configure.sh
# 1) gcc (on Linux) 5) Clang (on OSX)
# 2) No Debug Info
# 3) Yes (ROOT6)
# 4) Yes (Install engines)
# 5) Internet (install G4 files from internet)
# 6) No (without python)
# path: ~/fair_install/FairSoftInst
```
BRANCH_NAME=oct17p4

### 2. Install [FairRoot](https://github.com/FairRootGroup/FairRoot)

```
# Set the shell variable SIMPATH to the installation directory
export SIMPATH=~/fair_install/FairSoftInst

cd ~/fair_install
git clone https://github.com/FairRootGroup/FairRoot.git
cd FairRoot
git checkout BRANCH_NAME
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX="~/fair_install/FairRootInst" ..
make
make install
```
BRANCH_NAME=v-17.10

### 3. Install [ExpertRoot](#)

```
# Set the shell variable FAIRROOTPATH to the FairRoot installation directory
export FAIRROOTPATH=~/fair_install/FairRootInst
cd ~
mkdir expertroot
cd expertroot
git clone https://github.com/ExpertRootGroup/er/ .
git checkout BRANCH_NAME
cd ../
mkdir build
cd build
cmake ../ -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make
```

### Experimental data preprocessing

If you need to handle a raw experimental data, please install the _Go4_ and _AccDAQ_

### Install [Go4](https://www.gsi.de/en/work/research/experiment_electronics/data_processing/data_analysis/the_go4_home_page.htm)

```
wget -O go4.tar.gz http://web-docs.gsi.de/~go4/software/download.php?http://web-docs.gsi.de/~go4/download/go4-6.0.0.tar.gz
tar xzf go4.tar.gz
rm go4.tar.gz
mv go4-6.0.0 go4
cd go4
source $SIMPATH/bin/thisroot.sh
make withqt=no -j4
```

### Install [AccDAQ](https://github.com/FLNR-JINR/ACCULINNA_go4_user_library)

```
git clone https://github.com/flnr-jinr/ACCULINNA_go4_user_library accdaq &&\
cd accdaq
git checkout master
mkdir build && cd build
source $SIMPATH/bin/thisroot.sh
source $GO4_PATH/go4login
cmake ../ -DCMAKE_INSTALL_PREFIX=<installtion directory path>
make install -j4
```


## CMake options

There will be cmake options that affect the build process.

* ```-DACCULINNA_GO4 = <acculina_go4_install_path_> ```

To use _ERDigibuilder_ to read experimental data from _AccDAQ_.
You should indicate in which directory the library was installed. 

# Initialize

Environment variables must be initialized in **every** terminal session.

```
source <path_to_build>/config.sh
```

Otherwise ER won't work.

# Run tests

To run the entire test suite:

```
cd build && cmake build . && ctest .
```

To run a specific test:

```
cd build && cmake build . && ctest -R exp1904_h7_sim
```

# Authors and contributors

**ER** is currently developed and mantained at [FLNR](http://flerovlab.jinr.ru/) and [LIT](https://lit.jinr.ru/en) by:

* [Vitaliy Schetinin](mailto:schetinin@jinr.ru)
* [Sergey Belogurov](mailto:belogurov@jinr.ru)
* [Mikhail Kozlov](mailto:kozlovmy@jinr.ru)
* [Vratislav Chudoba](mailto:chudoba@jinr.ru)
* [Egor Ovcharenko](mailto:eovchar@jinr.ru)
* [Ivan Muzalevskii](mailto:muzalevsky@jinr.ru)
* [Ilyas Satyshev](mailto:satyshev@jinr.ru)
* [Sofya Rymzhanova](mailto:rymzhanova@jinr.ru)
