#!/usr/bin/env python3
import re
import sys


def main():
    if len(sys.argv) < 2 or len(sys.argv) > 2:
        print("Usage: python3 zadatak4.py html_file", file=sys.stderr)
        exit(1)
    input_file = sys.argv[1]
    try:
        file = open(input_file, "r")
    except FileNotFoundError:
        print("Cannot open input file", file=sys.stderr)
        exit(1)
    page_data = file.read()
    file.close()

    books = re.findall('(?<=<li>|<LI>).*?(?=</li>|</LI>)', page_data)

    for book in books:
        book_data = book.split(",")
        author = book_data[0].strip(" ")
        title = book_data[1].strip(" ")[3:-4]
        publisher = book_data[2].strip(" ")
        year_tag = book_data[3].strip(" ")
        year = year_tag.split(" ")[0]
        tag = year_tag.split(" ")[1][1:-1]
        print("@book{{{},".format(tag))
        print('   author = "{}",'.format(author))
        print('   title = "{}",'.format(title))
        print('   publisher = "{}",'.format(publisher))
        print('   year = "{}"'.format(year))
        print("}")
        print("")


if __name__ == "__main__":
    main()
