import re


def clean_string(input_string):
    # Replace newline and tab characters with spaces
    cleaned_string = re.sub(r'[\n\t]+', ' ', input_string)
    # Remove duplicate spaces
    cleaned_string = re.sub(r'\s+', ' ', cleaned_string)
    return cleaned_string


def extract_chunks(input, output):
    allowed_characters = '.,!?()-"; '
    with open(input, 'r', encoding='utf-8') as input_file:
        content = input_file.read()
        content = clean_string(content)
    chunks = []
    chunk = ""
    for i in range(len(content)):
        char = content[i]
        if len(chunk) == 0:
            if char.isupper() or (char == '"' and content[i+1].isupper()):
                chunk += char
        else:
            if char in allowed_characters or char.isalpha():
                chunk += char
                if len(chunk) > 300 and (chunk.count('"')) % 2 == 0 and char in '.!?"':
                    # header = f"Len: {len(chunk)}, WordLen: {max(len(word) for word in chunk.split())}\n"
                    header = f"WordCount: {len(chunk.split())}\n"
                    chunks.append(header + chunk)
                    chunk = ""
            else:
                chunk = ""
    print(f"Number of chunks: {len(chunks)}")
    header = f"TextCount: {len(chunks)}\n"
    with open(output, 'w', encoding='utf-8') as output_file:
        output_file.write(header)
        output_file.write('\n\n'.join(chunks))


if __name__ == "__main__":
    input_filename = 'lotr.txt'  # Replace with your input file
    output_filename = 'lotr_chunks.txt'  # Replace with your output file
    extract_chunks(input_filename, output_filename)

