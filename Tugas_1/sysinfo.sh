#!/bin/bash

echo "
==================================================
            TUGAS 1 OS - KELOMPOK C06
==================================================
"
echo -e "Mengecek sistem... \n"

#batas atas dan bawah User ID biasa
UIDMAX=$(awk '/^UID_MAX/ { print $2 }' /etc/login.defs)
UIDMIN=$(awk '/^UID_MIN/ { print $2 }' /etc/login.defs)

#nama os dan versi kernel
os=$(awk -F'"' '/^PRETTY_NAME=/ {print $2}' /etc/os-release)
kernel_ver=$(uname -r)

#jumlah user biasa
userCount=$(awk -v min="$UIDMIN" -v max="$UIDMAX" -F':' ' $3 >= min && $3 <= max {count++} END {print count}' /etc/passwd)
userStatus="PASS"
if [ $userCount -le 0 ]; then
    userStatus="FAIL"
fi

#cek proses
processCount=$(ps -e | awk 'NR > 1 { count++ } END { print count }')
processesItem="Running"
processesStatus="PASS"

if [ $processCount -le 0 ]; then
    processesItem="Not running"
    processesStatus="FAIL"
fi

#cek virtualisasi
virtualitation=$(systemd-detect-virt)

if echo "$virtualitation" | grep -q "none"; then
    terdekteksiVirtual="Tidak terdetekksi virtual"
    virtualitationStatus="FAIL"
else
    terdekteksiVirtual="Terdetekksi ($virtualitation)"
    virtualitationStatus="PASS"
fi

#memory, load avarage dan banyak core
usedMem=$(free | awk '/Mem/ { print ($3 / $2) * 100}' | sed 's/,/./g')
loadAvg=$(cat /proc/loadavg | awk '{print $1}')
cores=$(nproc)


if [ ! -f "./resource_check" ]; then
    gcc resource_check.c -o resource_check
fi

metrikInfo=$(echo "$usedMem $loadAvg $cores" | ./resource_check)
memStatus=$(echo "$metrikInfo" | awk '{print $1}')
loadStatus=$(echo "$metrikInfo" | awk '{print $2}')

if echo "$memStatus" | grep -q "PASS"; then
    memDetail="Masih aman"
elif echo "$memStatus" | grep -q "WARN"; then 
    memDetail="Mulai penuh"
else
    memDetail="Kritis"
fi

if echo "$loadStatus" | grep -q "PASS"; then
    loadDetail="Masih aman"
elif echo "$loadStatus" | grep -q "WARN"; then 
    loadDetail="Mulai penuh"
else
    loadDetail="Kritis"
fi


#waktu berjalan
uptime=$(uptime -p | sed 's/^up //g')

#fitur tambahan: ip adress dan sisa storage
ipAddr=$(hostname -I | awk '{print $1}')

diskFree=$(df -h / | awk 'NR==2 {print $4 " free dari " $2}')


echo -e " OS/Kernel \t\t: $os (Kernel $kernel_ver)"
echo -e " Akun pengguna \t\t: $userCount akun"
echo -e " Proses berjalan \t: $processCount"
echo -e " Virtualisasi \t\t: $terdekteksiVirtual"

echo -e "\nMenghitung metrik varian kelompok..."

echo -e " Memory usage \t: $usedMem% \t[ $memStatus ]"
echo -e " Load average \t: $loadAvg  \t[ $loadStatus ]"
echo -e " Core \t\t: $cores"

echo -e "\nFitur tambahan:"

echo -e " Waktu aktif\t: $uptime"
echo -e " User aktif\t: $USER"
echo -e " Alamat IP\t: $ipAddr"
echo -e " Sisa Disk\t: $diskFree"


echo -e "
=================================================================================
                            TUGAS 1 OS - KELOMPOK C06
=================================================================================
+----------------+----------------------+--------+------------------------------+
| Check Category | Item                 | Status | Details                      |
+----------------+----------------------+--------+------------------------------+
| OS             | $os \t| PASS   | $kernel_ver \t\t|
| Users          | Regular accounts \t| $userStatus   | $userCount akun \t\t\t|
| Processes      | $processesItem \t\t| $processesStatus   | $processCount proses berjalan \t\t|
| Virtualization | $virtualitation \t\t| $virtualitationStatus   | $terdekteksiVirtual \t|
| Memori         | $usedMem% \t\t| $memStatus   | $memDetail \t\t\t|
| Load avarage   | $loadAvg \t\t| $loadStatus   | $loadDetail \t\t\t|
| IP Adress	 | $ipAddr \t| INFO   | Alamat IP VM \t\t\t|
| Storage	 | Free space \t\t| INFO   | $diskFree \t\t|
+----------------+----------------------+--------+------------------------------+
" > sysinfo_report.txt
