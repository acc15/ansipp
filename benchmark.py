#!/usr/bin/env python3

import argparse
import pathlib
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

import matplotlib.pyplot as plt
import matplotlib.ticker as plticker

# import numpy as np

build_dir = pathlib.Path("build/benchmark")
test_executable = build_dir / "ansipp_test"

def build():
    return bool(subprocess.run(["cmake", "--workflow", "--preset benchmark"]))

def get_report_file(name: str) -> pathlib.Path:
    return build_dir / "reports" / (re.sub("[: ]+", "_", name) + ".xml")

def run_benchmark(name: str, report: pathlib.Path):
    if not build():
        return False
    report.parent.mkdir(parents=True, exist_ok=True)
    with open(report, "w") as out:
        return bool(subprocess.run([test_executable, name, "--reporter", "xml", "--benchmark-samples", "40"], stdout=out))

def parse_report_params(params: str) -> dict[str, str]:
    d = {}
    for p in params.split(";"):
        pp = p.split("=", 1)
        d[pp[0]] = pp[1]
    return d

def parse_report(path: pathlib.Path):
    result: dict[str, tuple[list[float], list[float]]] = {}
    xml = ET.parse(path)
    for section in xml.findall("TestCase/Section"):
        section_name = section.attrib["name"]
        params = parse_report_params(section_name)
        for tc in section.findall("BenchmarkResults"):
            benchmark_name = tc.attrib["name"]
            y_value = float(tc.find("mean").attrib["value"])
            axis_data = result.setdefault(benchmark_name, ([], []))
            axis_data[0].append(float(params["x"]))
            axis_data[1].append(y_value)
    return result

def plot(report: pathlib.Path):
    ax = plt.subplots()[1]
    ax.set_xlabel("# of digits")
    ax.set_ylabel("nanos")
    ax.xaxis.set_major_locator(plticker.MultipleLocator(1))
    for k, v in parse_report(report).items():
        ax.plot(v[0], v[1], label = k)
    ax.legend()
    plt.show()

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("name")
    p.add_argument("-r", "--rerun", action="store_true")
    a = p.parse_args()
    report = get_report_file(a.name)
    if not report.exists() or a.rerun:
        if not run_benchmark(a.name, report):
            exit(1)
    plot(report)

