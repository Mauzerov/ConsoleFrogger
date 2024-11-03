from sys import argv
import re
from pprint import pprint

if len(argv) != 2:
    print(f"Usage: '{__name__}' <file_name>")
    exit(1)

functions = dict()
brackets = 0
name = ""

with open(argv[1], 'r') as f:
    content = f.read()

while content:
    offset = 1
    if brackets == 0:
        # NOTE: this doesn't work with returning function pointers
        if match := re.match(
            r"^([a-zA-Z_0-9]+?(?:\s*\*)*)" # return type
            r"\s+([A-Z0-9_a-z]+)\s*" # func name
            r"(\([^;=]+?\))\s*{", # arguments
            content
        ):
            brackets = 1
            offset = match.end()
            _, name, args = match.groups()
            functions[name] = 0
    else:
        functions[name] += 1
        if content[0] == '{':
            brackets += 1
        elif content[0] == '}':
            brackets -= 1

    content = content[offset:]

pprint(functions)