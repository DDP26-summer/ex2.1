# Suduku basics

# How to use this repo

```
tsmc65
git clone https://github.com/DDP26-summer/ex2.1
cp -r ex2.1/hw my_k5_proj
cp -r ex2.1/sw my_k5_proj
```
# How to run sim
on sim terminal
```
set_k5_terminal
launch_k5_sim sudx_basic
```

on app terminal
```
set_k5_terminal
launch_k5_app sudx_basic -asl sud_shared
```
and if you want to run with accelrator set flag XON by:
```
launch_k5_app sudx_basic -asl sud_shared -ccd1 XON
```