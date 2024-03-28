# Usage of this part of neutron wall investigation

## 1) create geometry files in macro/geo folder

### target

```bash
root -l -q create_target_10he_3h_steel_geo.C
```

results in creation of

```bash
er/geometry/target.3h_Steel.geo.root
```

### neutron wall

Commands

```bash
root -l -q create_ND_geo_exp1904_10he_8m.C 
root -l -q create_ND_geo_exp1904_10he_8m_al.C
root -l -q create_ND_geo_exp1904_10he_8m_vac.C
```

will creates three files:

```bash
er/geometry/ND.geo.exp1904.10he.8m.al.root
er/geometry/ND.geo.exp1904.10he.8m.root
er/geometry/ND.geo.exp1904.10he.8m.vac.root
```

## 2) run simulation

```bash
root -l -q sim_digi.C
```

finishing with

```bash
*** Error in `/opt/FairSoft/bin/root.exe': double free or corruption (out): 0x0000000005ec9850 ***
```

It seems to be very probably related to geometry. However, it we obtain three files as a result:

```bash
par.root
setup_sim_digi_8_1nNDAl.root
sim_digi_8_1nNDAl.root
```

## 3) run reconstruction

Go to reco folder and launch reco script:

```bash
cd reco
root -l -q reco_10he_exp.C 
```

You will obtain output with plenty of errors and warnings, however

```bash
Macro finished succesfully.
Output file writen:  sim_digi_8_1nNDAl.target.root
Parameter file writen ../par.root
Real time 2.34817 s, CPU time 2.29 s
```

resulting to root file:

```bash
sim_digi_8_1nNDAl.target.root
```

## 4) processing of simulation and reconstruction results

We need some comment what is doing this script

```bash
root -l AfterReco.C
```

It results to some canvases and file with tree

```bash
reco_sim_digi_8_1nNDAl.root
```

## 5) data analysis

not tested yet