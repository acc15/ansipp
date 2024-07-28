#!/usr/bin/env python3

import argparse
import pathlib
import re
import subprocess
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field

import matplotlib.pyplot as plt
import matplotlib.ticker as plticker

build_dir = pathlib.Path("build/benchmark")
test_executable = build_dir / "ansipp_test"

def build():
    subprocess.run(["cmake", "--workflow", "--preset benchmark"]).check_returncode()

def run_benchmark(name: str, report: pathlib.Path):
    subprocess.run([
        test_executable, name, 
        "--reporter", "xml", 
        "--benchmark-samples", "40",
        "--out", str(report)
    ]).check_returncode()

def get_report_file(name: str) -> pathlib.Path:
    return build_dir / "reports" / (re.sub("[: ]+", "_", name) + ".xml")

def parse_report_params(params: str) -> dict[str, str]:
    d = {}
    for p in params.split(";"):
        pp = p.split("=", 1)
        d[pp[0]] = pp[1]
    return d

@dataclass
class Report:
    xlabel: str = ""
    xtick: float | None = None
    data: dict[str, tuple[list[float], list[float]]] = field(default_factory=dict)

def parse_report(path: pathlib.Path) -> Report:
    result = Report()
    for section in ET.parse(path).findall("TestCase/Section"):
        section_name = section.attrib["name"]
        params = parse_report_params(section_name)
        if "xlabel" in params: 
            result.xlabel = params["xlabel"]
        if "xtick" in params: 
            result.xtick = float(params["xtick"])
        x_value = float(params["x"])
        for tc in section.findall("BenchmarkResults"):
            benchmark_name = tc.attrib["name"]
            y_value = float(tc.find("mean").attrib["value"])
            data = result.data.setdefault(benchmark_name, ([], []))
            data[0].append(x_value)
            data[1].append(y_value)
    return result

def plot(report: Report):
    ax = plt.subplots()[1]
    if report.xlabel:
        ax.set_xlabel(report.xlabel)
    ax.set_ylabel("nanos")
    if report.xtick is not None:
        ax.xaxis.set_major_locator(plticker.MultipleLocator(report.xtick))
    for k, v in report.data.items():
        ax.plot(v[0], v[1], label = k)
    ax.legend()
    plt.show()

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("name")
    p.add_argument("-r", "--rerun", action="store_true")
    a = p.parse_args()
    report_file = get_report_file(a.name)
    if not report_file.exists() or a.rerun:
        build()
        run_benchmark(a.name, report_file)
    plot(parse_report(report_file))

