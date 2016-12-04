
def get_data(lines):
    import re 
    data, s_cov, p_cov, msmt_cov, a, f = [], [], [], [], [], []
    a_active, f_active, m_cov_active, s_cov_active, p_cov_active = False, False, False, False, False
    idx = -1
    prefix = ""
    seen = set([])
    for count in range(len(lines)): 
        line = lines[count].rstrip()
        if not m_cov_active and not p_cov_active and not s_cov_active and not a_active and not f_active:
            prefix = line.split("=")[0] + ","
            idx = str(line.split("=")[0][-1])
            if prefix in seen:
                continue
            else:
                seen.add(prefix)
        if line.startswith("ms_"):
            ms_str = line.split("=[")[1][:-1]
            ms_toks = filter(lambda x: x != '', re.split(" +", ms_str))
            assert len(ms_toks) <= 15
            data.append(prefix  + ",".join(ms_toks))
        if line.startswith("s_"):
            state_str = line.split("=[")[1][:-1]
            state_toks = filter(lambda x: x != '', re.split(" +", state_str))
            assert len(state_toks) == 15
            data.append(prefix  + ",".join(state_toks))
        if line.startswith("d_"):
            data.append(prefix + line.split("=")[1])            
        if line.startswith("m_"):
            msmt_str = line.split("=[")[1][:-1]
            msmt_toks = filter(lambda x: x!= '', re.split(" +", msmt_str))
            assert len(msmt_toks) == 15
            data.append(prefix  + ",".join(msmt_toks))
        if line.startswith("a_"):
            a_active = True
            a_toks =  re.split(" +", line.split("=[")[1])
            a.append(a_toks)
        elif a_active is True:    
            if line.endswith("]"):
                a_toks =  re.split(" +", line[:-1])
                a.append(a_toks)            
                a_flattened = []
                [[ a_flattened.append(item) for item in cov if item]  for cov in a]
                assert len(a_flattened) == 225
                data.append(prefix + ",".join(a_flattened))
                a = []
                a_active = False
            else:   
                a_toks =  re.split(" +", line)
                a.append(a_toks)

        if line.startswith("f_"):
            f_active = True
            f_toks =  re.split(" +", line.split("=[")[1])
            f.append(f_toks)
        elif f_active is True:    
            if line.endswith("]"):
                f_toks =  re.split(" +", line[:-1])
                f.append(f_toks)            
                f_flattened = []
                [[ f_flattened.append(item) for item in cov if item]  for cov in f]
                assert len(f_flattened) == 225
                data.append(prefix + ",".join(f_flattened))
                f = []
                f_active = False
            else:   
                f_toks =  re.split(" +", line)
                f.append(f_toks)
        if line.startswith("r_"):
            m_cov_active = True
            m_cov_toks =  re.split(" +", line.split("=[")[1])
            msmt_cov.append(m_cov_toks)
        elif m_cov_active is True:    
            if line.endswith("]"):
                m_cov_toks =  re.split(" +", line[:-1])
                msmt_cov.append(m_cov_toks)            
                msmt_cov_flattened = []
                [[ msmt_cov_flattened.append(item) for item in cov if item]  for cov in msmt_cov]
                assert len(msmt_cov_flattened) == 225
                data.append(prefix + ",".join(msmt_cov_flattened))
                msmt_cov = []
                m_cov_active = False
            else:   
                m_cov_toks =  re.split(" +", line)
                msmt_cov.append(m_cov_toks)
        if line.startswith("q_"):
            p_cov_active = True
            p_cov_toks = re.split(" +", line.split("=[")[1])
            p_cov.append(p_cov_toks)
        elif p_cov_active is True:    
            if line.endswith("]"):
                p_cov_toks = re.split(" +", line[:-1])
                p_cov.append(p_cov_toks)
                p_cov_flattened = []
                [[ p_cov_flattened.append(item) for item in cov if item]  for cov in p_cov]
                assert len(p_cov_flattened) == 225
                data.append(prefix +  ",".join(p_cov_flattened))
                p_cov = []
                p_cov_active = False
            else:   
                p_cov_toks =  re.split(" +", line)
                p_cov.append(p_cov_toks)
        if line.startswith("p_"):
            s_cov_active = True
            s_cov_toks = re.split(" +", line.split("=[")[1])
            s_cov.append(s_cov_toks)
        elif s_cov_active is True:    
            if line.endswith("]"): 
                s_cov_toks = re.split(" +", line[:-1])
                s_cov.append(s_cov_toks)
                s_cov_flattened = []
                [[ s_cov_flattened.append(item) for item in cov if item]  for cov in s_cov]
                assert len(s_cov_flattened) == 225
                data.append(prefix  + ",".join(s_cov_flattened))
                s_cov = []
                s_cov_active = False
            else:   
                s_cov_toks = re.split(" +", line)
                s_cov.append(s_cov_toks)
    return data

def get_data_small(data):
    data_small = []
    for line in data:
        if line.startswith("d_k-1"):
            data_small.append(line.replace("d_k-1+_", "0,"))
        elif line.startswith("m_k-_"):
            data_small.append(line.replace("m_k-_", "1,"))
        elif line.startswith("ms_k-_"):
            data_small.append(line.replace("ms_k-_", "2,"))  
        elif line.startswith("r_k-_"):
            data_small.append(line.replace("r_k-_", "3,"))
    return data_small

def get_predict_data(data):
    data_predict = []
    for line in data:
        if line.startswith("s_k-1+_"):
            data_predict.append(line.replace("s_k-1+_", "0,"))
        elif line.startswith("p_k-1+_"):
            data_predict.append(line.replace("p_k-1+_", "1,"))
        elif line.startswith("q_k-1+_"):
            data_predict.append(line.replace("q_k-1+_", "2,"))
        elif line.startswith("d_k-1+_"):
            data_predict.append(line.replace("d_k-1+_", "3,"))
    return data_predict

def get_update_data(data):
    data_u = {}
    for line in data: 
        idx = int((line.split("_")[-1]).split(",")[0])
        if line.startswith("s_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("s_k-_", "0,"))  
            else: 
                data_u[idx] = [line.replace("s_k-_", "0,")]
        if line.startswith("p_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("p_k-_", "1,"))  
            else: 
                data_u[idx] = [line.replace("p_k-_", "1,")]
        if line.startswith("q_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("q_k-_", "2,"))  
            else: 
                data_u[idx] = [line.replace("q_k-_", "2,")]
        if line.startswith("d_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("d_k-_", "3,"))  
            else: 
                data_u[idx] = [line.replace("d_k-_", "3,")]
        if line.startswith("m_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("m_k-_", "4,"))  
            else: 
                data_u[idx] = [line.replace("m_k-_", "4,")]
        if line.startswith("ms_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("ms_k-_", "5,"))  
            else: 
                data_u[idx] = [line.replace("ms_k-_", "5,")]
        if line.startswith("r_k-_"):
            if idx in data_u:
                data_u[idx].append(line.replace("r_k-_", "6,"))  
            else: 
                data_u[idx] = [line.replace("r_k-_", "6,")]
    return data_u

def usage():
    print ("python compare_out.py <data_folder>")
    exit()


if "__main__" == __name__:
    import os, sys

    if len(sys.argv) != 2:
        usage()

    data_folder = sys.argv[1]

    inp = os.path.join(data_folder, "inp_test.txt")
    out1 = os.path.join(data_folder, "out_test.txt")
    out2 = os.path.join(data_folder, "out_compact_test.txt")
    out3 = os.path.join(data_folder, "out_predict_test.txt")
    out4 = os.path.join(data_folder, "out_update_test.txt")

    lines = open(inp, "r").readlines()
    data = get_data(lines)    
    m = open(out1, "w")
    for i in  range(len(data)):
        m.write(data[i] + "\n")
    m.close()

    data_small = get_data_small(data)
    m_small = open(out2, "w")
    for i in  range(len(data_small)):
        m_small.write(data_small[i] + "\n")
    m_small.close()

    data_predict = get_predict_data(data)
    p_test = open(out3, "w")
    for i in  range(len(data_predict)):
        p_test.write(data_predict[i] + "\n")
    p_test.close()

    data_update = get_update_data(data)
    u_test = open(out4, "w")
    for key in sorted(data_update.keys()):
        if (len(data_update[key]) == 7):
            for val in data_update[key]:
                u_test.write(str(val) + "\n")
    u_test.close()
    
