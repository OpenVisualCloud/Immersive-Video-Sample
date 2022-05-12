## User guidance

### Run an client simulator instance
```bash
./sync_time.sh # modify the target url first
./run.sh
```
### Run multiple client simulator instances
```bash
./sync_time.sh # modify the target url first
# mkdir test01 test02 ... testxx
# cp run.sh ClientSimulator test01 ... testxx
cd test01
./run.sh
cd test02
./run.sh
...
```
### Check the latency data
```bash
vim data.csv # refer to last line to show the max/min/avg latency
```
