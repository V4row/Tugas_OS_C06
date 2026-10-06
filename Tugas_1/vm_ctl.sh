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

pause() {
    print_header
    nama_vm=$@
    echo "Men-suspend VM '$nama_vm' ..."
    if [ -z "$nama_vm" ]; then
        echo "ERROR: Masukkan nama VM. Contoh: $0 stop <nama_vm>"
        exit 1
    fi


    VBoxManage controlvm "$nama_vm" pause > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        # while VBoxManage list runningvms | grep -q "\"$nama_vm\""; do
        #     sleep 1
        # done

        echo "VM '$nama_vm' berhasil di pause. Status: paused"
    else
        echo "ERROR: VM '$nama_vm' tidak sedang berjalan atau tidak terdaftar"
        exit 1
    fi
    echo ""
}

resume() {
    print_header
    nama_vm=$@
    echo "Men-suspend VM '$nama_vm' ..."
    if [ -z "$nama_vm" ]; then
        echo "ERROR: Masukkan nama VM. Contoh: $0 resume <nama_vm>"
        exit 1
    fi


    VBoxManage controlvm "$nama_vm" resume > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        # while VBoxManage list runningvms | grep -q "\"$nama_vm\""; do
        #     sleep 1
        # done

        echo "VM '$nama_vm' berhasil di resume. Status: running"
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
    pause)
	shift 1
        nama_vm=$@
        pause $nama_vm
	;;
    resume)
	shift 1
        nama_vm=$@
        resume $nama_vm
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
