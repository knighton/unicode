def each_line(file_name):
    with open(file_name) as f:
        for line in f:
            x = line.find('#')
            if x != -1:
                line = line[:x]
            if not line or line.isspace():
                continue
            yield line.rstrip()
