#import os
input_file_name = 'results.txt'
output_file_name = 'results_converted_RGB.nit'

def process_table(text_file, out):
    line = text_file.readline()
    while line != '\n' and line != '':
        tokens = line.split()
        out.append(list(map(float, tokens)))
        line = text_file.readline()

data = []
wl_400 = []
wl_500 = []
wl_600 = []

with open(input_file_name) as text_file:
    line = text_file.readline()
    while line != '':
        tokens = line.split()
        if len(tokens) == 0:
            line = text_file.readline()
            continue
        if tokens[0] == 'wavelength' and tokens[1] == 'r':
            process_table(text_file, wl_400)
        if tokens[0] == 'wavelength' and tokens[1] == 'g':
            process_table(text_file, wl_500)
        if tokens[0] == 'wavelength' and tokens[1] == 'b':
            process_table(text_file, wl_600)
        line = text_file.readline()

r = wl_400
g = wl_500
b = wl_600

pp = PostProcessor(PPDataUnits.LUMINANCE, [], r, g, b)

pp.SaveToHDR(output_file_name, overwrite = OverwriteMode.OVERWRITE)
