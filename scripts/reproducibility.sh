#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o freighttiger
./freighttiger data/shipment_records.csv data/context_notes.csv run1.csv 2> /dev/null
./freighttiger data/shipment_records.csv data/context_notes.csv run2.csv 2> /dev/null
./freighttiger data/shipment_records.csv data/context_notes.csv run3.csv 2> /dev/null
cmp run1.csv run2.csv
cmp run1.csv run3.csv
cp run1.csv output.csv
rm run1.csv run2.csv run3.csv
echo "PASS: three runs produced byte-identical CSV output."
