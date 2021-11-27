def main():
    input_string = "xbidlo01"
    add = 2

    for c in input_string:
        o = ord(c)
        o += add
        add = -9 if (add == 2) else 2

        if o > ord('z'):
            o = ord("a") + (o - ord("z"))
        elif o < ord('a'):
            o = ord("z") - (ord("a") - o - 1)

        if c.isdigit():
            break

        print(chr(o))


if __name__ == '__main__':
    main()
