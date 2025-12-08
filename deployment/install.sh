#!/bin/bash
set -e

echo "Instalador - Detector de Somnolencia"
echo "====================================="

if ! ping -c 1 -W 2 10.203.207.1 > /dev/null 2>&1; then
    echo "Error: No se puede conectar a MaixCAM"
    exit 1
fi

if [ ! -f "../build/libdrowsiness.so" ]; then
    echo "Error: Librería no compilada"
    exit 1
fi

echo "Transfiriendo archivos..."
scp ../build/libdrowsiness.so root@10.203.207.1:/root/
scp -r ../python/drowsiness_detector root@10.203.207.1:/maixapp/apps/
scp -r ../python/menu_selector root@10.203.207.1:/maixapp/apps/
scp S06maixapp root@10.203.207.1:/etc/init.d/
ssh root@10.203.207.1 'chmod +x /etc/init.d/S06maixapp'
ssh root@10.203.207.1 'reboot' || true

echo "Instalación completada"
