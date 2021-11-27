def main():
    input_string = "xdobro23"
    add = 4

    for c in input_string:
        o = ord(c)
        o += add
        add = -15 if (add == 4) else 4

        if o > ord('z'):
            o -= 26
        elif o < ord('a'):
            o += 26

        if c.isdigit():
            break

        print(chr(o))


if __name__ == '__main__':
    main()
