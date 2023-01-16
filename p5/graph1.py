
import sys
import os
import time
import matplotlib.pyplot as plt
import math

# associtivity range
assoc_range = [1, 2, 4]
# block size range
bsize_range = [x for x in range(2, 15)]
# capacity range
cap_range = [c for c in range(10, 22)]
# number of cores (1, 2, 4)
cores = [1, 2, 4]
# coherence protocol: (none, vi, or msi)
protocol=['none', 'vi', 'msi']

stats = ["n_cpu_accesses", "n_loads", "n_stores", "n_hits", "n_misses", "hit_rate", "miss_rate", "n_upgrade_miss", "n_bus_snoops", "n_snoop_hits", "n_writebacks", "B_written_bus_to_cache", "B_written_cache_to_bus_wb", "B_written_cache_to_bus_wt", "B_total_traffic_wb", "B_total_traffic_wt"]

expname='exp1'


def get_stats(logfile, key):
    for line in open(logfile):
        if line[2:].startswith(key):
            line = line.split()
            return float(line[1])
    return 0

def run_exp(logfile, protocol, core, cap, bsize, assoc):
    trace = 'trace.%dt.long.txt' % core
    cmd="./p5 -t %s -p %s -n %d -cache %d %d %d >> %s" % (
            trace, protocol, core, cap, bsize, assoc, logfile)
    print(cmd)
    os.system(cmd)

def graph():
    timestr = time.strftime("%m.%d-%H_%M_%S")
    folder = "results/"+expname+"/"+timestr+"/"
    os.system(f"mkdir -p {folder}/graphs")

    stats_calc = {
        p:{
            d:{
                a:{
                    b:{
                        c:None for c in cap_range
                    } for b in bsize_range
                } for a in assoc_range
            } for d in cores
        } for p in protocol
    }

    # for p in protocol:
    #     for d in cores:
    #         for a in assoc_range:
    #             for b in bsize_range:
    #                 for c in cap_range:
    #                     if c >= math.frexp(b-1)[1] + a:
    #                         logfile = folder+"%s-%02d-%02d-%02d-%02d.out" % (
    #                                 p, d, c, b, a)
    #                         run_exp(logfile, p, d, c, b, a)
    #                         stats_calc[p][d][a][b][c] = dict((x, get_stats(logfile, x)) for x in stats)


    def plot_2_indep(p, d, a, b, c, perm=(0,1), key="miss_rate", scale=lambda x: x, ylim=(None, None), prefix="", keep=False):
        def gen_tup(x, y):
            l = [x, y]
            t = [p, d, a, b, c]
            idx = 0
            for ti in range(5):
                if isinstance(t[ti], list):
                    t[ti] = l[perm[idx]]
                    idx += 1
            return t

        tln = ["protocol", "cores", "associativity", "block size", "capacity"]
        tlax = [0, 0, 0, 1, 1]
        tl = (p, d, a, b, c)

        pl = []

        for i in range(5):
            if isinstance(tl[i], list):
                pl.append(i)

        plots = []

        for x in tl[pl[perm[0]]]:
            k = []
            v = []
            for y in tl[pl[perm[1]]]:
                (pi, di, ai, bi, ci) = gen_tup(x, y)
                if stats_calc[pi][di][ai][bi][ci] is None:
                    logfile = folder+"%s-%02d-%02d-%02d-%02d.out" % (
                                    pi, di, ci, bi, ai)
                    run_exp(logfile, pi, di, ci, bi, ai)
                    stats_calc[pi][di][ai][bi][ci] = dict((x, get_stats(logfile, x)) for x in stats)
                k.append((1 << y) if tlax[pl[perm[1]]] else y)
                v.append(scale(stats_calc[pi][di][ai][bi][ci][key]))
            pp,=plt.plot(k, v, label=f'{prefix}{tln[pl[perm[0]]]} {(1 << x) if tlax[pl[perm[0]]] else x}')
            plots.append(pp)

        plt.legend()
        if tlax[pl[perm[1]]]:
            plt.xscale('log', base=2)
        plt.title(f'{tln[pl[perm[1]]]} vs {key}')
        plt.xlabel(tln[pl[perm[1]]])
        plt.ylabel(key)
        if ylim[0] is not None:
            plt.ylim(bottom=ylim[0])
        if ylim[1] is not None:
            plt.ylim(top=ylim[1])
        if not keep:
            plt.savefig(f"{folder}/graphs/{'_'.join([str(x) for x in gen_tup('N','N')])}_{key}.png")
            plt.clf()


    plots = []

    # Plot assoc vs cap

    plot_2_indep('none', 1, [1, 2, 4], 6, [x for x in range(10, 22)], key="miss_rate", scale=lambda x:x/100, ylim=(0, None))
    plot_2_indep('none', 1, [1, 2, 4], 6, [x for x in range(10, 22)], key="B_written_cache_to_bus_wb", ylim=(0, None))
    plot_2_indep('vi', [2, 4], 4, [x for x in range(2, 15)], 16, key="miss_rate", scale=lambda x:x/100, ylim=(0, None), prefix="VI ", keep=True)
    plot_2_indep('none', [1], 4, [x for x in range(2, 15)], 16, key="miss_rate", scale=lambda x:x/100, ylim=(0, None), prefix="none ")
    plot_2_indep('msi', [1, 2, 4], 4, [x for x in range(2, 15)], 16, key="miss_rate", scale=lambda x:x/100, ylim=(0, None), prefix="MSI ")

graph()
