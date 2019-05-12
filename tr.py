#!/bin/python3

from matplotlib import pyplot as plt
import numpy as np

names = [
    'amptjp-bal.rep',
    #'amptjp.rep',
    'binary2-bal.rep',
    #'binary2.rep',
    'binary-bal.rep',
    #'binary.rep',
    'cccp-bal.rep',
    #'cccp.rep',
    'coalescing-bal.rep', # very big chungus
    #'coalescing.rep',
    'cp-decl-bal.rep',
    #'cp-decl.rep',
    'expr-bal.rep',
    #'expr.rep',
    'random2-bal.rep',
    #'random2.rep',
    'random-bal.rep',
    #'random.rep',
    'realloc2-bal.rep',
    #'realloc2.rep',
    'realloc-bal.rep',
    #'realloc.rep',
    'short1-bal.rep',
    #'short1.rep',
    'short2-bal.rep',
    #'short2.rep',
    ]

for n in names:
    x = 0
    m = {}
    h = []
    sh = []
    fig, (ax1,ax2) = plt.subplots(2)
    for l in open('traces/'+n):
        l = l.split()
        if l[0] == 'a':
            i,s = int(l[1]),int(l[2])
            x += s
            m[i] = s
            sh.append(s)
        elif l[0] == 'f':
            i = int(l[1])
            x -= m[i]
        elif l[0] == 'r':
            i,s = int(l[1]),int(l[2])
            x += s - m[i]
            m[i] = s
            sh.append(s)
        h.append(x)
    fig.suptitle(n)
    ax1.hist(sh,bins=10**np.linspace(np.log10(16), np.log10(64000), 50))
    ax1.set_xscale('log')
    ax2.plot(h)
    plt.show()

