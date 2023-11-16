import re


def clean_string(input_string):
    # Replace newline and tab characters with spaces
    input_string = re.sub(r'[\n\t]+', ' ', input_string)
    # Replace ` with ' if present
    input_string = input_string.replace('`', "'")
    # Remove duplicate spaces
    output_string = re.sub(r'\s+', ' ', input_string)
    return output_string


def extract_chunks(input, output_long, output_short):
    allowed_characters = ['.', ",", "!", "?", ";", " ", "'"]
    with open(input, 'r', encoding='utf-8') as input_file:
        content = input_file.read()
        content = clean_string(content)
    print(f"Max word length: {max([len(word) for word in content.split()])}")
    chunks = []
    chunk = ""
    for i in range(len(content)):
        char = content[i]
        if len(chunk) == 0:
            if char.isupper():
                chunk += char
        else:
            if char in allowed_characters or char.isalpha():
                chunk += char
                if len(chunk) > 100 and char in ".!?":
                    # header = f"Len: {len(chunk)}, WordLen: {max(len(word) for word in chunk.split())}\n"
                    header = f"WordCount: {len(chunk.split())}\n"
                    chunks.append(header + chunk)
                    chunk = ""
            else:
                chunk = ""
    print(f"Number of chunks: {len(chunks)}")
    header = f"TextCount: {len(chunks)}\n"
    with open(output_long, 'w', encoding='utf-8') as output_file:
        output_file.write(header)
        output_file.write('\n\n'.join(chunks))
    header = f"TextCount: 100\n"
    with open(output_short, 'w', encoding='utf-8') as output_file:
        output_file.write(header)
        output_file.write('\n\n'.join(chunks[:100]))

if __name__ == "__main__":
    input_filename = 'hobbit.txt'  # Replace with your input file
    output_long = '../HF3/hobbit_long.txt'  # Replace with your output file
    output_short = '../HF3/hobbit_short.txt'
    extract_chunks(input_filename, output_long, output_short)

