import sys

threshold = 0.01

idx_label =    \
{              \
    0:  "x",   \
    1:  "y",   \
    2:  "z",   \
    3:  "r",   \
    4:  "p",   \
    5:  "yw",  \
    6:  "vx",  \
    7:  "vy",  \
    8:  "vz",  \
    9:  "vr",  \
    10: "vp",  \
    11: "vyw", \
    12: "ax",  \
    13: "ay",  \
    14: "az"   \
}


key_order =   \
[             \
    "s_k-1+", \
    "p_k-1+", \
    "q_k-1+", \
    "a_k-1+", \
    "f_k-1+", \
    "d_k-1+", \
    "s_k-",   \
    "p_k-",   \
    "q_k-",   \
    "a_k-",   \
    "f_k-",   \
    "d_k-",   \
    "m_k-",   \
    "ms_k-",   \
    "r_k-",   \
    "s_k+",   \
    "p_k+",   \
    "q_k+",   \
    "a_k+",   \
    "f_k+",   \
    "d-k+",   \
    "m_k+",   \
    "ms_k+",  \
    "r_k+"    \
]

pred_comp_keys =    \
{                   \
    "s_k-1+": True, \
    "p_k-1+": True, \
    "q_k-1+": True, \
    "a_k-1+": False, \
    "f_k-1+": False, \
    "d_k-1+": True, \
    "s_k-": True,  \
    "p_k-": True,   \
    "q_k-": True,  \
    "a_k-": False,  \
    "f_k-": False,  \
    "d_k-": False,  \
    "m_k-": False,  \
    "ms_k-": False,  \
    "r_k-": False,  \
    "s_k+": False,  \
    "p_k+": False,  \
    "q_k+": False,  \
    "a_k+": False,  \
    "f_k+": False,  \
    "d_k+": False,  \
    "m_k+": False,  \
    "ms_k+": False,  \
    "r_k+": False   \
}

corr_comp_keys =     \
{                   \
    "s_k-1+": False, \
    "p_k-1+": False, \
    "q_k-1+": False, \
    "a_k-1+": False, \
    "f_k-1+": False, \
    "d_k-1+": False, \
    "s_k-": True,  \
    "p_k-": True,   \
    "q_k-": True,  \
    "a_k-": False,  \
    "f_k-": False,  \
    "d_k-": False,  \
    "m_k-": True,  \
    "ms_k-": True,  \
    "r_k-": True,  \
    "s_k+": True,  \
    "p_k+": True,  \
    "q_k+": True,  \
    "a_k+": False,  \
    "f_k+": False,  \
    "d_k+": False,  \
    "m_k+": False,  \
    "ms_k+": False,  \
    "r_k+": False   \
}

def get_label(num, max_len):
    if max_len > 15:
        return (idx_label[int(num/15)], idx_label[int(num%15)])
    else :
        return (idx_label[int(num%15)], idx_label[int(num%15)])

def process_out(out1, comp_keys):

    data, c_data, p_idx = {}, {}, -1
    lines = open(out1, "r").readlines()    
    for line in lines:
        line = line.rstrip()
        if not line:    # ignore empty lines
            continue

        prefix_idx = line.split(",")[0]
        prefix = "_".join(prefix_idx.split("_")[0:2])

        if not comp_keys[prefix]: # compare only needed keys
            continue

        idx = int(prefix_idx.split("_")[-1])
        if idx != p_idx:
            if p_idx != -1:
                data[p_idx] = c_data
            c_data = {}
            p_idx = idx
        
        value = filter(lambda x: x != "", line.split(",")[1:])
        c_data[prefix] = value

    return data


def compare(data1, data2, idx_min, idx_max):

    keys1, keys2 = sorted(data1.keys()), sorted(data2.keys())
    assert (len(keys1) == len(keys2) and keys1[0] == keys2[0] and keys1[-1] == keys2[-1])
    for key in keys1:
        if int(key) < idx_min:
            continue
        elif int(key) > idx_max:
            break
        
        pfxs1, pfxs2 = sorted(data1[key].keys()), sorted(data2[key].keys())
        if (not (len(pfxs1) == len(pfxs2) and pfxs1[0] == pfxs2[0] and pfxs1[-1] == pfxs2[-1])):
            print pfxs1, pfxs2
        assert (len(pfxs1) == len(pfxs2) and pfxs1[0] == pfxs2[0] and pfxs1[-1] == pfxs2[-1]);

        for pfx in pfxs1:
            val1 = data1[key][pfx]
            val2 = data2[key][pfx]             
            sim = reduce(lambda x, y: x & y, [(abs(float(item1)-float(item2)) <= threshold ) for item1, item2 in zip(val1, val2)])
            if not sim:
                print "Not equal : ", key, ",",  pfx, "\n"
                print "EKF    EKFN\n"
                label_idx = 0
                for v1, v2 in zip(val1, val2):
                     label = get_label(label_idx, len(val1))
                     if (float(v1) != 0 or float(v2) != 0):                    
                         if (abs(float(v1)-float(v2)) > threshold ) :
                             print label[0] + "_" + label[1] + ",", v1, ",", v2, "X"
                         else:
                             print label[0] + "_" + label[1] + ",", v1, ",", v2
                     label_idx += 1
                
                print ("Exiting ... ", key)     
                exit()

def usage():
    print ("python compare_out.py <data_folder>")
    exit()


if "__main__" == __name__:
    import os

    if len(sys.argv) != 2:
        usage()

    data_folder = sys.argv[1]
    out1 = os.path.join(data_folder, "../data/out_test.txt")

    print "Compare predict output ...  ", 
    out2 = os.path.join(data_folder, "../data/out_se_predict.txt")
    data1 = process_out(out1, pred_comp_keys)
    data2 = process_out(out2, pred_comp_keys)
    if len(data1.keys()) < len(data2.keys()):
        data2 = dict(filter(lambda (key, value) : key in set(data1.keys()) , data2.items()))
    elif len(data2.keys()) < len(data1.keys()):
        data1 = dict(filter(lambda (key, value) : key in set(data2.keys()) , data1.items()))

    assert (len(data1.keys()) == len(data2.keys()))
    compare(data1, data2, 0, len(data1.keys()))
    print "[OK]"
  
    print "Compare update output ... ", 
    out2 = os.path.join(data_folder, "../data/out_se_update.txt")
    data1 = process_out(out1, corr_comp_keys)
    data2 = process_out(out2, corr_comp_keys)
    if len(data1.keys()) < len(data2.keys()):
        data2 = dict(filter(lambda (key, value) : key in set(data1.keys()) , data2.items()))
    elif len(data2.keys()) < len(data1.keys()):
        data1 = dict(filter(lambda (key, value) : key in set(data2.keys()) , data1.items()))

    assert (len(data1.keys()) == len(data2.keys()))
    compare(data1, data2, 0, len(data1.keys()))
    print "[OK]"

    
