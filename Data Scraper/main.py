# reads through the output files from GecodeExtension to turn into csv format
import os
import csv

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

# turn file into dict with useful info
def parse_file(filename:str) -> dict:
    with open(filename, "r") as file:
        data = {"timeout" : False}
        for line in file:
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


#initializes the csv file
def init_csv_file():
    with open(csv_filename, "w", newline="") as csvfile:
        writer = csv.DictWriter(csvfile, csv_cols)
        writer.writeheader()


# writes data to csv file as a line
def append_to_csv(data:dict) -> str:
    with open(csv_filename, "a", newline="") as csvfile:
        writer = csv.DictWriter(csvfile, csv_cols)
        writer.writerow(data)

# main, loops through files
def main(directory:str):
    init_csv_file()
    for filename in os.listdir(directory):
        match filename.split("_"):
            # LOG_<solutions>_<TestType>_<domain increases>_<id>_<propagator>.txt
            case ["LOG", sols, ("B" | "X" | "R") as test, dom, num, ("AdvModulo.txt" | "Modulo.txt" | "Linear.txt") as prop]:
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
    file = os.path.abspath(os.path.join(os.getcwd(), "..\GecodeExtension\Out"))
    main(file)