### TUGAS 1

[`vm_ctl.sh`](https://github.com/V4row/Tugas1_C06/blob/main/vm_ctl.sh):
```bash
#!/bin/bash

list() {
    print_header
    echo "Memindai daftar Virtual Machine..."
    echo ""
    echo "Daftar VM terdaftar:"
    VBoxManage list vms | awk -F'"' '{print NR ". " $2}'
    echo ""
}

info() {
    print_header
    nama_vm=$@
    
    info_arr=( $(VBoxManage showvminfo "$nama_vm" 2>&1 | grep -E "Memory size|Number of CPUs|State" | awk -F'[:()]' '{print $2}' | xargs))
    
    if [ "${#info_arr[@]}" -eq 0 ]; then
	echo "Info VM gagal ditampilkan. Pastikan nama VM benar."
    else
	echo "VM			: $nama_vm"

    	ramUse=${info_arr[0]}
    	echo "RAM dialokasikan	: $ramUse"

    	vCPU=${info_arr[1]}
    	echo "vCPU dialokasikan	: $vCPU"

    	state="${info_arr[2]} ${info_arr[3]}"
    	echo "Status saat ini		: $state"
    fi
    echo ""
}

start() {
    print_header
    nama_vm=$@
    if [ -n "$nama_vm" ]; then
        echo "Menyalakan VM '$nama_vm' secara headless..."
        VBoxManage startvm "$nama_vm" --type headless > /dev/null 2>&1
        
        if [ $? -eq 0 ]; then
            echo "VM '$nama_vm' berhasil dinyalakan. Status: running"
        else
            echo "Gagal menyalakan VM '$nama_vm'."
            exit 1 
        fi

    else
        echo "ERROR: Masukkan nama VM. Contoh: $0 start <nama_vm>"
        exit 1
    fi
    echo ""
}

stop() {
    print_header
    nama_vm=$@
    echo "Mematikan VM '$nama_vm' secara aman..."
    if [ -z "$nama_vm" ]; then
        echo "ERROR: Masukkan nama VM. Contoh: $0 stop <nama_vm>"
        exit 1
    fi


    VBoxManage controlvm "$nama_vm" acpipowerbutton > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        while VBoxManage list runningvms | grep -q "\"$nama_vm\""; do
            sleep 1
        done

        echo "VM '$nama_vm' berhasil dimatikan. Status: powered off"
    else
        echo "ERROR: VM '$nama_vm' tidak sedang berjalan atau tidak terdaftar"
        exit 1
    fi
    echo ""
}

snapshot_create() {
    print_header
    args=("$@")
    args_count="${#args[@]}"
    nama_snapshot="${args[args_count-1]}"
    nama_vm="${args[*]:0:args_count-1}"
    
    echo "Membuat snapshot '$nama_snapshot' pada VM '$nama_vm'..."
    timestamp=$(date "+%Y-%m-%d_%H:%M:%S")

    VBoxManage snapshot "$nama_vm" take "${nama_snapshot}_${timestamp}" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "Snapshot '$nama_snapshot' berhasil dibuat pada $timestamp"
    else
	echo "Snapshot '$nama_snapshot' gagal dibuat. Pastikan nama VM benar."
    fi
    echo ""
}

snapshot_list() {
    print_header
    nama_vm=$@
    count=1
    echo "Daftar snapshot VM '$nama_vm':"

    while IFS= read -r line; do
    if [[ "$line" =~ Name:[[:space:]]+([^[:space:]]+) ]]; then
        current_name="${BASH_REMATCH[1]}"
	echo "$count. $current_name"
	count=$((count+1))
    fi
    done < <(VBoxManage snapshot "$nama_vm" list 2>&1)

    if [ $? -ne 0 ]; then
	echo "Daftar snapshot gagal dicetak. Pastikan nama VM benar."
    fi
    echo ""
}

print_header() {
    echo "==================================="
    echo "     TUGAS 1 OS - KELOMPOK C06"
    echo "==================================="
}

case "$1" in
    list)
        list
	;;
    info)
	shift 1
        nama_vm=$@
        info $nama_vm
	;;
    start)
	shift 1
        nama_vm=$@
        start $nama_vm
	;;
    stop)
	shift 1
        nama_vm=$@
        stop $nama_vm
	;;
    snapshot)
	case "$2" in
	    create)
		args=("$@")
		args_count="${#args[@]}"
		nama_snapshot="${args[args_count-1]}"
		nama_vm="${args[*]:2:args_count-3}"

		snapshot_create $nama_vm $nama_snapshot
		;;
	    list)
		shift 2
		nama_vm=$@
		snapshot_list $nama_vm
		;;
	    *)
		echo "Input tidak valid"
		echo ""
		;;
	esac
	;;
    *)
	echo "Input tidak valid"
	echo ""
	;;
esac

```


[`sysinfo.sh`](https://github.com/V4row/Tugas1_C06/blob/main/sysinfo.sh):
```bash
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

```

[`resource_check.c`](https://github.com/V4row/Tugas1_C06/blob/main/resource_check.c):

```C
#include <stdio.h>
#include <stdlib.h>

int main() {
    float mem_usage;
    float load_average;
    int core_count;

    if (scanf("%f %f %d", &mem_usage, &load_average, &core_count) != 3) {
        return 1;
    }


    if (mem_usage >= 90) {
        printf("FAIL ");
    } else if (mem_usage >= 75) {
        printf("WARN ");
    } else {
        printf("PASS ");
    }

    if (load_average > (2.0 * core_count)) {
        printf("FAIL\n");
    } else if (load_average > (1.0 * core_count)) {
        printf("WARN\n");
    } else {
        printf("PASS\n");
    }

    return 0;
}
```

Jangan lupa memberi hak eksekusi:
```bash
chmod 764 vm_ctl.sh
chmod 764 sysinfo.sh
```
dan mengcompile:
```bash
gcc resource_check.c -o resource_check
```


# Laporan
Dapat dilihat disini ==> [**Laporan kelompok**](https://docs.google.com/document/d/1IlYTej52JhAYZh2aTkJCX-sl-_wn4tJA6DWUzRtCcAc/edit?usp=sharing) <==
