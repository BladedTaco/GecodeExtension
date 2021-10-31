# reads through the output files from GecodeExtension to turn into csv format
import os
import csv
import json

csv_cols = [
    "propagator type",
    "test type",
    "domain group",
    "test id",
    "timeout",
    "runtime",
    "requested solutions",
    "solutions",
    "propagations",
    "nodes",
    "failures",
    "peak depth"
]

csv_filename = "output.csv"
json_filename = "output.json"


# turn file into dict with useful info
def parse_file(filename: str) -> dict:
    with open(filename, "r") as infile:
        data = {"timeout": False}
        for line in infile:
            match line.strip().split(':'):
                case ["runtime" as key, time]:
                    data[key] = float(time.strip().split(" ")[0])
                case [("solutions" | "propagations" | "nodes" | "failures" | "peak depth") as key, value]:
                    data[key] = int(value.strip())
                case ["reason", " time limit reached"]:
                    data["timeout"] = True
                case _:
                    pass
    return data


# initializes the csv file
def init_csv_file():
    with open(csv_filename, "w", newline="") as csv_file:
        writer = csv.DictWriter(csv_file, csv_cols)
        writer.writeheader()
    with open(json_filename, "w", newline="") as json_file:
        json_file.write("[")


# writes data to csv file as a line
def append_to_csv(data: dict):
    with open(csv_filename, "a", newline="") as csv_file:
        writer = csv.DictWriter(csv_file, csv_cols)
        writer.writerow(data)
    # writes data to json file as well, the last comma needs to be replaced with a ]
    with open(json_filename, "a", newline="") as json_file:
        json.dump(data, json_file, indent=4)
        json_file.write(",")



# main, loops through files
def main(directory: str):
    init_csv_file()
    for filename in os.listdir(directory):
        match filename.split("_"):
            # LOG_<solutions>_<TestType>_<domain increases>_<id>_<propagator>.txt
            case [
                "LOG", sols, ("B" | "X" | "R") as test, dom, num,
                ("AdvModulo.txt" | "Modulo.txt" | "Linear.txt") as prop
            ]:
                data = parse_file(directory + "\\" + filename) | {
                    "requested solutions": int(sols),
                    "test type": test,
                    "domain group": int(dom),
                    "test id":  int(num),
                    "propagator type": prop.removesuffix(".txt")
                }
                append_to_csv(data)
            case _:
                print(f"please clean up the output directory, '{filename}' shouldn't be here.")


if __name__ == '__main__':
    file = os.path.abspath(os.path.join(os.getcwd(), "..\\GecodeExtension\\Out"))
    main(file)
