#!/usr/bin/env python3
"""Score a candidate cycles.csv against a baseline cycles.csv.

    scripts/geomean.py --baseline results/zbb/cycles.csv --candidate results/cand/cycles.csv

Score = geometric mean over kernels of cycles(candidate) / cycles(baseline).
Lower is better; 1.0 means parity with the baseline.

A row is valid only if it completed: for the CVA6 runner this means
rvfi_term != NONE and status == SUCCESS; for the Chipyard runner status == PASS.
If any kernel present in the baseline lacks a valid candidate row, the
candidate is unscored and the script exits with status 2.
"""
import argparse
import csv
import math
import sys

KERNELS = ["adpcm", "aes", "basicmath", "bitcount", "crc32", "dijkstra", "fft",
           "fir_filter", "matmul", "qsort", "sha", "sha256_full", "stringsearch"]


def load(path):
    out = {}
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            k = row.get("kernel")
            try:
                cyc = int(row.get("cycles", ""))
            except ValueError:
                continue
            ok = row.get("status") in ("SUCCESS", "PASS") and row.get("rvfi_term", "x") != "NONE" and cyc > 0
            if ok:
                out[k] = cyc
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", required=True)
    ap.add_argument("--candidate", required=True)
    ap.add_argument("--kernels", nargs="*", default=KERNELS)
    a = ap.parse_args()
    base, cand = load(a.baseline), load(a.candidate)
    missing = [k for k in a.kernels if k not in base]
    if missing:
        print(f"baseline lacks valid rows for: {' '.join(missing)}", file=sys.stderr)
        return 2
    invalid = [k for k in a.kernels if k not in cand]
    print(f"{'kernel':<14}{'baseline':>14}{'candidate':>14}{'ratio':>10}")
    logs = []
    for k in a.kernels:
        if k in cand:
            r = cand[k] / base[k]
            logs.append(math.log(r))
            print(f"{k:<14}{base[k]:>14,}{cand[k]:>14,}{r:>10.4f}")
        else:
            print(f"{k:<14}{base[k]:>14,}{'INVALID':>14}{'-':>10}")
    if invalid:
        print(f"UNSCORED: no valid candidate result for {' '.join(invalid)}", file=sys.stderr)
        return 2
    g = math.exp(sum(logs) / len(logs))
    print(f"{'geomean':<14}{'':>14}{'':>14}{g:>10.4f}   ({(g-1)*100:+.2f}% cycles vs baseline)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
