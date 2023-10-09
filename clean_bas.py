import re

with open('bas.txt', 'r') as f:
    text = f.read()

lines = text.split('\n')
text = ''
unique_lines = {}
clean_text = ""

# Compile regex pattern to match 4-7 spaces followed by a number or a minus
pattern = re.compile(r'^ {4,7}[-]?\d')

for i, line in enumerate(lines):
    print("\r{}/{}".format(i, len(lines)), end='', flush=True)
    if len(line) == 0:
        continue
    # Skip lines matching the pattern
    if pattern.match(line):
        continue
    if line in unique_lines:
        unique_lines[line] += 1
    else:
        unique_lines[line] = 1
    if unique_lines[line] >= 20:
        continue
    if "📈" in line:
        continue
    if "📉" in line:
        continue

    clean_text += line + "\n"

with open('bas_clean.txt', 'w') as f:
    f.write(clean_text)
