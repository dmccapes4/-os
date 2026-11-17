#!/bin/sh
# lirvol_processing_check.sh — run ON lirvol (via ssh) before it takes
# major processing duty. Read-only except CPU load: a 120 s all-core
# SHA-256 burn while temperature, throttling, and fan witnesses are
# sampled. Output is the receipt; the bench files it.
# Usage (from bench): ssh lirvol@<ip> 'sh -s' < tools/lirvol_processing_check.sh
set -u

echo "== identity =="
hostname; cat /proc/device-tree/model; echo
grep Serial /proc/cpuinfo
uname -a
echo "== before =="
vcgencmd get_throttled; vcgencmd measure_temp; uptime

echo "== fan witnesses =="
ls /sys/class/thermal/ 2>/dev/null
for d in /sys/class/hwmon/hwmon*; do
  echo "$d name=$(cat "$d/name" 2>/dev/null) fan1=$(cat "$d/fan1_input" 2>/dev/null) pwm1=$(cat "$d/pwm1" 2>/dev/null)"
done
for c in /sys/class/thermal/cooling_device*/; do
  [ -d "$c" ] && echo "$c type=$(cat "$c/type" 2>/dev/null) cur=$(cat "$c/cur_state" 2>/dev/null) max=$(cat "$c/max_state" 2>/dev/null)"
done
find /proc/device-tree -maxdepth 1 -iname "*fan*" 2>/dev/null

echo "== nvme =="
lsblk -o NAME,SIZE,MODEL,TRAN,FSTYPE,MOUNTPOINT /dev/nvme0n1 2>/dev/null
cat /sys/class/nvme/nvme0/model /sys/class/nvme/nvme0/serial 2>/dev/null
for z in /sys/class/thermal/thermal_zone*/; do
  echo "$z $(cat "$z/type" 2>/dev/null) $(cat "$z/temp" 2>/dev/null)"
done
df -h / /var/lib/kyrios/ledger 2>/dev/null

echo "== stress: 120 s all-core sha256, sampling every 5 s =="
NCPU="$(nproc)"
python3 - <<'PY' &
import hashlib, os, time
data = os.urandom(1024 * 1024)
end = time.time() + 120
n = 0
while time.time() < end:
    hashlib.sha256(data).digest(); n += 1
print(f"stress worker done: {n} MiB-sha rounds")
PY
STRESS=$!
# one worker per core
i=1
while [ "$i" -lt "$NCPU" ]; do
python3 - <<'PY' &
import hashlib, os, time
data = os.urandom(1024 * 1024)
end = time.time() + 120
while time.time() < end:
    hashlib.sha256(data).digest()
PY
  i=$((i + 1))
done

t=0
while [ "$t" -lt 120 ]; do
  TEMP="$(vcgencmd measure_temp 2>/dev/null)"
  FAN=""
  for d in /sys/class/hwmon/hwmon*; do
    f="$(cat "$d/fan1_input" 2>/dev/null)"
    [ -n "$f" ] && FAN="$FAN $(basename "$d")=$f"
  done
  echo "t=${t}s $TEMP fan:${FAN:- none-registered} load:$(cut -d' ' -f1-3 /proc/loadavg)"
  sleep 5
  t=$((t + 5))
done
wait $STRESS 2>/dev/null

echo "== after =="
vcgencmd measure_temp; vcgencmd get_throttled
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq 2>/dev/null
free -h | head -2
echo "PROCESSING-CHECK-DONE"
