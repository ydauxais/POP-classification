import numpy as np
import subprocess
import sys
import itertools
import os
import timeit
import copy

from argparse import ArgumentParser

from sklearn.ensemble import RandomForestClassifier
from sklearn.svm import SVC
from sklearn.tree import DecisionTreeClassifier

sys.path.append("../RPO/SWIG/rpo")
import rpo

#########################
#                       #
#       Dataset         #
#                       #
#########################


class Dataset:
    def __init__(self, item_mapping=[]):
        self.item_mapping = item_mapping
        self.reverse_item_mapping = {}
        self.db = {}
        self.db_seq = {}
        self.max_occ = {}
        self.group = {}
        self.inv_group = {}

    def get_features(self, episodes, match_functor, train=None):
        X = []
        y = []

        for file_id in range(len(self.db.keys())):
            label = list(self.db.keys())[file_id]
            for sid in range(len(self.db_seq[label])):
                x = []
                for episode in episodes:
                    if episode.is_valid():
                        v, n = match_functor(self.get_vertical_sequence(label, sid), episode, train)
                        x += [v, n]
                X.append(x)
                y.append(file_id)

        return X, y
        
    def get_vertical_sequence(self, label, sid):
        return [item[sid] for item in self.db[label]] + [[] for _ in range(len(self.db[label]), len(self.item_mapping))]

    def shuffle(self):
        for label in self.db:
            permut = np.random.permutation(len(self.db[label][0]))
            self.db[label] = [np.array(item)[permut.tolist()].tolist() for item in self.db[label]]
            if len(self.db_seq) > 0:
                self.db_seq[label] = np.array(self.db_seq[label])[permut.tolist()].tolist()

    def load_vertical_sequences(self, fi, label):
        if label not in self.db:
            self.db[label] = []
        with open(fi, 'r') as fin:
            nbseq = 0
            max_occ = 0
            db = self.db[label]
            for line in fin:
                lline = line.split()
                if len(lline) > 0 and len(lline) % 2 == 0:
                    for i in range(0, len(lline), 2):
                        item = lline[i]
                        time = int(lline[i + 1])

                        if not item in self.reverse_item_mapping:
                            self.reverse_item_mapping[item] = len(self.item_mapping)
                            self.item_mapping.append(item)
                        it = self.reverse_item_mapping[item]

                        for _ in range(len(db), len(self.item_mapping)):
                            db.append([])
                        for _ in range(len(db[it]), nbseq + 1):
                            db[it].append([])
                        db[it][nbseq].append(time)
                        if len(db[it][nbseq]) > max_occ:
                            max_occ = len(db[it][nbseq])
                    nbseq += 1
            for item in db:
                for _ in range(len(item), nbseq):
                    item.append([])
            self.max_occ[label] = max_occ

    def load_sequences(self, filename, label):
        if label not in self.db_seq:
            self.db_seq[label] = []
        with open(filename, 'r') as fin:
            for line in fin:
                l = line.replace("\n", "")
                sequence = l.split(" ")
                self.db_seq[label].append([(e, int(t))
                                           for e, t in zip(sequence[0::2], sequence[1::2])
                                           ])

    def occurrences(self, label, sid, multiset, return_first=False):
        occurrences = []
        for item in multiset:
            item_type = item[0]
            item_id = self.reverse_item_mapping[item_type]
            if item_id < len(self.db[label]) and len(self.db[label][item_id][sid]) >= item[1]:
                occurrences.append(self.db[label][item_id][sid])
                if return_first:
                    return occurrences
            else:
                return []
        return occurrences

    def write_sequences(self, filename, label, tid_list=None, mode='w', multiset=None):
        with open(filename, mode) as fout:
            if multiset:
                tlist = tid_list
                if not tid_list:
                    tlist = list(range(len(self.db_seq[label])))
                for tid in tlist:
                    seq = []
                    invalid_seq = False
                    for item, card in multiset:
                        item_id = self.reverse_item_mapping[item]
                        if item_id < len(self.db[label]) and len(self.db[label][item_id][tid]) >= card:
                            seq += [(item, t) for t in self.db[label][item_id][tid]]
                        else:
                            invalid_seq = True
                            break
                    if not invalid_seq:
                        fout.write(" ".join([str(e) + " " + str(t) for (e, t) in seq]) + "\n")
            else:
                tlist = tid_list
                if not tid_list:
                    tlist = list(range(len(self.db_seq[label])))
                for tid in tlist:
                    fout.write(" ".join([str(e) + " " + str(t) for (e, t) in self.db_seq[label][tid]]) + "\n")
    
#########################
#                       #
#    Data management    #
#                       #
#########################

def get_fold_sets(dataset, i):
    train_base = Dataset(dataset.item_mapping)
    train_base.max_occ = dataset.max_occ
    test_base = Dataset(dataset.item_mapping)
    test_base.max_occ = dataset.max_occ
    train_base.reverse_item_mapping = dataset.reverse_item_mapping
    test_base.reverse_item_mapping = dataset.reverse_item_mapping
    set_fold_sets(dataset, train_base, test_base, i)
    return train_base, test_base

def set_fold_sets(dataset, train_base, test_base, i):
    global args
    
    for label in dataset.db:
        threshold = int(len(dataset.db[label][0]) * (1.0/args.k))
        if i == 0:
            train_base.db[label] = [item[threshold:] for item in dataset.db[label]]
            train_base.db_seq[label] = dataset.db_seq[label][threshold:]

            test_base.db[label] = [item[:threshold] for item in dataset.db[label]]
            test_base.db_seq[label] = dataset.db_seq[label][:threshold]
        else:
            train_base.db[label] = [item[:i * threshold] + item[(i + 1) * threshold:] for item in dataset.db[label]]
            train_base.db_seq[label] = dataset.db_seq[label][:i * threshold] + dataset.db_seq[label][
                                                                               (i + 1) * threshold:]

            test_base.db[label] = [item[i * threshold:(i + 1) * threshold] for item in dataset.db[label]]
            test_base.db_seq[label] = dataset.db_seq[label][i * threshold:(i + 1) * threshold]
    
#########################
#                       #
#       Parameters      #
#                       #
#########################

def episode_mining_options():
    global args
    
    opt = rpo.CPEngineOptions("rpo")
    opt.set_solutions(0)
    opt.closed_ = args.closed
    opt.output_tid_lists_ = False
    opt.minimal_length_ = args.mins
    opt.maximal_length_ = args.maxs
    opt.no_output()
    opt.set_export_episodes()
    opt.minimal_growth_rate_ = args.gmin
    opt.relevant_ = args.relevant
    opt.numeric_intervals_ = args.numeric_intervals

    rpo.init_database(opt)
    
    return opt

        
def parse_arguments():
    pars = ArgumentParser()
    pars.add_argument("input")
    pars.add_argument("--ignored_label", dest="ignored_label")
    pars.add_argument("--chronicle", dest="numeric_intervals", action="store_true")
    pars.add_argument("--all_closed", dest="relevant", action="store_false")
    pars.add_argument("--no_closed_constraint", dest="closed", action="store_false")
    pars.add_argument("--min_size", dest="mins", type=int, default=0)
    pars.add_argument("--max_size", dest="maxs", type=int, default=0)
    pars.add_argument("--kfold", dest="k", type=int, default=5)
    pars.add_argument("--fmin", dest="fmin", type=float, default=0.2)
    pars.add_argument("--gmin", dest="gmin", type=float, default=0)
    pars.add_argument("--classifier", dest="classifier", default="SVC")
    pars.add_argument("--out", dest="out", default="")
    return pars.parse_args(sys.argv[1:])
    
    
def output(text, out):
    if out:
        with open(out, 'a') as fout:
            fout.write(text+"\n")
    else:
        print(text)


#########################
#                       #
#         Match         #
#                       #
#########################

    
def match(sequence, episode, train=None):
    n = episode.occurs_in_sequence(rpo.UMatrix(sequence), True)    
    return n > 0, n
        
#########################
#                       #
#          Main         #
#                       #
#########################

args = None

def main():
    global args
    args = parse_arguments()
    
    dataset = Dataset()
    for file in os.listdir(args.input):
        if file.endswith(".dat"):
            dataset.load_vertical_sequences(args.input + "/" + file, file.split(".dat")[0])
            dataset.load_sequences(args.input + "/" + file, file.split(".dat")[0])
    dataset.shuffle()

    if not os.path.isfile(args.out):
        output("dataset,method,classifier,fold,kfold,fmin,gmin,max_size,svc_C,pattern_type,closed_multiset,accuracy,pattern_number,extraction_time,time", args.out)
    dataset_name = args.input.split("/")[-1]
    
    for i in range(args.k):
        train_base, test_base = get_fold_sets(dataset, i)
        extraction_time = 0
        episodes = []

        global_start = timeit.default_timer()
        for file in train_base.db:
            opt = episode_mining_options()

            pos = rpo.ClassVerticalDatabase(train_base.db[file])
            neg = [[x for other in train_base.db if not other == file
                      and len(train_base.db[other]) > item
                      for x in train_base.db[other][item]]
                        for item in range(len(train_base.db[file]))]

            max_seq_num = max([len(item) for item in neg])
            for x in range(len(neg)):
                for k in range(len(neg[x]), max_seq_num):
                    neg[x].append([])

            neg = rpo.ClassVerticalDatabase(neg)

            rpo.set_item_mapping(opt, rpo.StringVector(train_base.item_mapping))
            rpo.set_positives(opt, pos)
            rpo.set_negatives(opt, neg)
            
            opt.minimal_support_ = int(len(train_base.db[file][0]) * args.fmin)
            start = timeit.default_timer()
            print("Run: " +file)
            rpo.run(opt)
            episodes += opt.get_episodes()
            stop = timeit.default_timer()
            dif_time = stop - start
            extraction_time += dif_time
        
        X, y = train_base.get_features(episodes, match, train_base)
        X_test, y_test = test_base.get_features(episodes, match, train_base)
        first_global_time = timeit.default_timer() - global_start
        
        c_list = [1]
        if args.classifier in ["SVC", "SVM"]:
            c_list = [0.001, 0.01, 0.1, 1, 10, 100, 1000]
        for c in c_list:
            classif_start = timeit.default_timer()
            print("Train")
            
            if args.classifier in ["dt", "DT", "DecisionTree"]:
                clf = DecisionTreeClassifier()
            elif args.classifier in ["RF"]:
                clf = RandomForestClassifier(n_estimators=2000)
            else:
                clf = SVC(C=c)
            clf.fit(X, y)

            print("Test")
            acc = clf.score(X_test, y_test)
            
            print("Acc: " + str(acc))
            global_time = first_global_time + timeit.default_timer() - classif_start

            pattern_type = "relevant"
            if not args.relevant:
                pattern_type = "closed"
                
            output(",".join(
                [dataset_name, "occurrence", str(args.classifier), str(i), str(args.k), str(args.fmin), str(args.gmin), str(args.maxs), str(c), pattern_type, str(args.closed), str(acc),
                 str(len(X[0])), str(extraction_time), str(global_time)]),
                args.out)

if __name__ == "__main__":
    main()
