#!/usr/bin/python3

from string import Formatter
from os import path, urandom, mkdir
from shaaa import SHAAA
from pathlib import Path

#Join Formatter: Custom formatter for condensing a list via "string".join()
'''
W_SUMS = [1,2,3]
f"w_mem[${W_SUMS:join|] + w_mem[}];"
        v
w_mem[1] + w_mem[2] + w_mem[3];
'''
class JoinFormatter(Formatter):
    def format_field(self, value, format_spec):
        if format_spec.startswith('join|'):
            params = format_spec.split('|')
            return params[1].join(map(str,value))
        else:
            return super().format_field(value, format_spec)


#Lookup Formatter: Custom formatter optimized for making case statement lookup tables.
'''
Create a lookup table given an input list of values. Keys are the (0-based) indexes.
f"${valueslist:lookup|[Fill1]|[Fill2]}
<key0>[Fill1]<value0>[Fill2]<key1>[Fill1]<value1>[Fill2]...
'''
class LookupFormatter(JoinFormatter):
    def format_field(self, values, format_spec):
        if format_spec.startswith('lookup|'):
            (_lookup, fill1, fill2) = format_spec.split('|')
            return ''.join([str(i)+fill1+values[i]+fill2 for i in range(len(values))])
        elif format_spec.startswith('lookup_d|'): #With decimal formatting
            (_lookup, fill1, fill2) = format_spec.split('|')
            return ''.join([str(i)+fill1+int(values[i])+fill2 for i in range(len(values))])
        elif format_spec.startswith('lookup_x|'): #With hexadecimal formatting
            (_lookup, fill1, fill2) = format_spec.split('|')
            return ''.join([str(i)+fill1+hex(values[i])[2:]+fill2 for i in range(len(values))])
        else:
            return super().format_field(values, format_spec)


def generate_testbenches(shaaa, hashes, hashlen):
    blocks = []
    results = []
    controls = []
    for _i in range(hashes):
        m = urandom(hashlen)
        shaaa.digest(m)
        blocks.extend( map(lambda x:x.hex(), shaaa.last_blocks) )
        results.extend( map(lambda x:x.hex(), shaaa.last_intermediate_results) )

        l = len(shaaa.last_blocks)
        controls.extend( ["2"] + ["1"]*(l-1) ) #Init on first, "next" on each following

    samples=map(lambda x:'_'.join(x), zip(controls, blocks, results))
    return list(samples)

def generate_random_config(WORDSIZE):
    BLOCKSIZE=WORDSIZE*16

    p_sigma_low = random.sample(range(1,17), 2)
    w_sums = random.sample(range(1,17), random.randrange(2,5)) # random sampling of [1,16], between 2 and 4 selected them.
    iterations = random.randrange(64,128) #"Real" default is 64 or 80. Randomizing between 64 and 127.

    config={
        "WORDSIZE":       WORDSIZE,
        "BLOCKSIZE":      BLOCKSIZE,
        "ITERATIONS":     iterations,
        "H0":             gen_random_list_bits(8,WORDSIZE),
        "K":              gen_random_list_bits(iterations,WORDSIZE),
        "SIGMA_CAP_0":    gen_random_list_value(3,WORDSIZE),
        "SIGMA_CAP_1":    gen_random_list_value(3,WORDSIZE),
        "SIGMA_LOW_0":    gen_random_list_value(3,WORDSIZE),
        "SIGMA_LOW_1":    gen_random_list_value(3,WORDSIZE),
        "W_SUMS":         w_sums, 
        "P_SIGMA_LOW_0":  p_sigma_low[0],
        "P_SIGMA_LOW_1":  p_sigma_low[1],
        "DIGEST_BITS":    DIGEST_BITS
    }
    return config

#Generate a list of n random ints with _bits_ random bits.
def gen_random_list_bits(n, bits):
    return [secrets.randbits(bits) for x in range(n)]

#Generate a list of n random ints, each in range [0, max_value)
def gen_random_list_value(n,max_value):
    return [secrets.randbelow(max_value) for x in range(n)]

desc = """
Generate a SHA(AA) module, both verilog implementation and testbenches, based
on the specified configuration parameters.

Either start with a fully-specified configuration file, 
or randomly generate a new configuration sized to match SHA256 or SHA512.
"""
if __name__ == "__main__":
    import argparse
    import math
    import secrets
    import random
    import json

    parser = argparse.ArgumentParser(description=desc, 
                                    formatter_class=argparse.RawDescriptionHelpFormatter)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--config", "-c", 
            help="Load a JSON-formatted configuration file for testbench generation",
            type=argparse.FileType('r', encoding='UTF-8'),
            default=None)
    group.add_argument("--random256", 
            help="Generate a new randomized SHAAA module, with wordsize, blocksize, and digestsize equivalent to SHA256",
            action="store_true")
    group.add_argument("--random512", 
            help="Generate a new randomized SHAAA module, with wordsize, blocksize, and digestsize equivalent to SHA512",
            action="store_true")
            
    parser.add_argument("--module-name",
            type=str,
            default="shaaa",
            help="Module name override - set the base module name for all generated verilog. Only applies to randomly generated modules, will not override the name in the json configuration.")

    parser.add_argument("--LLKI", 
            help="Note: Only partially implemented. Currently generates <shaaa>_mock_tss.sv. For use with integrating to the CEP.",
            action="store_true")
    parser.add_argument("--output","--generated-output-directory","-o",
            type=lambda p: Path(p).resolve(),
            default=Path(__file__).resolve().parent / "generated",
            help="Output directory")
    parser.add_argument("--no-git-ignore",
            action="store_true",
            help="Prevent creation of a wildcard .gitignore in the output directory. Will only create the wildcard .gitignore if it doesn't yet exist.")

    parser.add_argument("--num-hashes",
            type=int,
            help="Number of hashes to calculate in the testbench",
            default="20")
    parser.add_argument("--hash-data-len",
            type=int,
            help="Input data length for each testbench hash, in bits.",
            default="300")
    args = parser.parse_args()

    if args.config is not None:
        config = json.load(args.config)
    else: 
        if args.random256:
            WORDSIZE=32
            DIGEST_BITS=256
        elif args.random512:
            WORDSIZE=64
            DIGEST_BITS=512

        config = generate_random_config(WORDSIZE)
        config["MODULE_NAME"] = args.module_name


    args.output.mkdir(exist_ok=True) #Make sure output directory exists.
    with open(args.output/(config["MODULE_NAME"]+"_config.json"), 'w') as configfile:
        json.dump(config, configfile, indent=4)
    print("Configuration:")
    print(config)
    
    #Testbench generation
    shaaa = SHAAA(config)
    samples = generate_testbenches(shaaa, args.num_hashes, args.hash_data_len)

    fmt = LookupFormatter()
    templatespath = Path(__file__).resolve().parent / "templates"

    #Formatting of data:
    config["LOG_ITERATIONS"] = math.ceil(math.log2(config["ITERATIONS"]))
    config["SHAAA_STIMULUS_LENGTH"] = len(samples)
    config["SHAAA_STIMULUS_DATA"] = "\n   ,'h".join(samples)

    #Verilog file generation
    #filelist has "shaaa" prepended by default, but will be renamed as needed by config["MODULE_NAME"]
    #Template files all have "t" appended to file extension name
    filelist = [".v", "_w_mem.v", "_k_constants.v", "_tb.sv", "_stimulus.v"]
    if args.LLKI:
        filelist += ["_mock_tss.sv"]
    for filename in filelist:
        with open(templatespath  / ("shaaa"+filename+"t"), 'r') as template:
            with open(args.output / (config["MODULE_NAME"]+filename), 'w') as generated:
                generated.write(fmt.format(template.read(), **config))

    if not args.no_git_ignore:
        try:
            with open(args.output / ".gitignore", 'x') as gitignore:
                #Don't overwrite the file if it already exists!
                gitignore.write("*\n")
        except:
            pass #.gitignore already existed.
    
    print("\nSuccess!")