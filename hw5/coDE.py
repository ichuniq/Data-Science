import numpy as np
import random
from random import random, uniform
import math
# from random import sample
# must use python 3.6, 3.7, 3.8(3.8 not for macOS) for sourcedefender
import sourcedefender
from HomeworkFramework import Function

class DE_optimizer(Function):
    def __init__(self, target_func, pop_size=5):
        super().__init__(target_func)

        self.lower = self.f.lower(target_func)
        self.upper = self.f.upper(target_func)
        self.dim = self.f.dimension(target_func)
        self.target_func = target_func

        self.eval_times = 0
        self.optimal_value = float("inf")
        self.optimal_solution = np.empty(self.dim)
        
        self.mut_pool = [1, 1, 0.8, 0.8]
        self.cr_pool = [0.1, 0.9, 0.2, 0.5]
        self.pop_size = 4+math.floor(2.5*math.log(self.dim))

        # initialize population
        self.pop = np.asarray([[uniform(self.lower, self.upper) for j in range(self.dim)] for i in range(self.pop_size)])

    def get_optimal(self):
        return self.optimal_solution, self.optimal_value

    def rand1bin(self, j, id, func_num):
        idxs = [idx for idx in range(self.pop_size) if idx != j]
        #------Mutation------
        a, b, c = self.pop[np.random.choice(idxs, 3, replace = False)]
        mutant = a + self.mut_pool[id] * (b - c)
        mutant = np.clip(mutant, self.lower, self.upper)
        #------Recombination------
        cross_points = np.random.rand(self.dim) < self.cr_pool[id]
        if np.random.randint(0, self.dim) == j:
            cross_points[j] = True
        if not np.any(cross_points):
            cross_points[np.random.randint(0, self.dim)] = True
        #------Selection------
        trial = np.where(cross_points==True, mutant, self.pop[j])
        score_trail = self.f.evaluate(func_num, trial)
        self.eval_times += 1
        return trial, score_trail

    def rand2bin(self, j, id, func_num):
        idxs = [idx for idx in range(self.pop_size)]
        #------Mutation------
        a, b, c, d, e = self.pop[np.random.choice(idxs, 5, replace = False)]
        mutant = a + self.mut_pool[id] * (b - c) + self.mut_pool[id] * (d - e)
        mutant = np.clip(mutant, self.lower, self.upper)
        #------Recombination------
        cross_points = np.random.rand(self.dim) < self.cr_pool[id]
        if np.random.randint(0, self.dim) == j:
            cross_points[j] = True
        if not np.any(cross_points):
            cross_points[np.random.randint(0, self.dim)] = True
        #------Selection------
        trial = np.where(cross_points==True, mutant, self.pop[j])
        score_trail = self.f.evaluate(func_num, trial)
        self.eval_times += 1
        return trial, score_trail

    def cur_to_rand1(self, j, id, func_num):
        idxs = [idx for idx in range(self.pop_size)]
        #------Mutation------
        a, b, c = self.pop[np.random.choice(idxs, 3, replace = False)]
        mutant = self.pop[j] + uniform(0,1) * ((a - self.pop[j]) + self.mut_pool[id] * (b - c))
        mutant = np.clip(mutant, self.lower, self.upper)
        trial = mutant
        score_trail = self.f.evaluate(func_num, trial)
        self.eval_times += 1
        return trial, score_trail



    def run(self, FES):
        # initialize fitness
        fitness = np.asarray([self.f.evaluate(func_num, row) for row in self.pop])
        self.eval_times += self.pop_size
        best_idx = np.argmin(fitness)
        self.optimal_solution[:] = self.pop[best_idx]

        while self.eval_times <= FES:
            print("=====eval_times: ", self.eval_times)
            for j in range(self.pop_size):
                rand_i = np.random.randint(0, 4)
                if func_num==1:
                    rand_i = 3
                elif func_num==2:
                    rand_i = 3

                trial1, score1 = self.rand1bin(j, rand_i, func_num)
                trial2, score2 = self.rand2bin(j, rand_i, func_num)
                trial3, score3 = self.cur_to_rand1(j, rand_i, func_num)
                
                S = [score1, score2, score3]
                T = [trial1, trial2, trial3]

                for s in S:
                    if s == "ReachFunctionLimit":
                        print("Reach Function Limit")
                        return

                S = [float(s) for s in S]

                if min(S) == 0:
                    print("===Something went wrong!!!===")
                    continue
                else:
                    score_trial = min(S) 
                    si = S.index(score_trial)
                    trial = T[si]
                
                if (score_trial < fitness[j]):
                    si = S.index(score_trial)
                    # print("===func {} is used".format(si+1))
                    fitness[j] = score_trial
                    self.pop[j] = trial
                    if (score_trial < fitness[best_idx]):
                        best_idx = j
                        # print("----new optimal: {}".format(score_trial))

            self.optimal_value = fitness[best_idx]
            self.optimal_solution[:] = self.pop[best_idx]
            print("optimal: {}".format(self.get_optimal()[1]))




if __name__ == '__main__':
    func_num = 1
    fes = 0 # number constraint for calling function
    #function1: 1000, function2: 1500, function3: 2000, function4: 2500
    while func_num  < 5:
        if func_num == 1:
            fes = 1000
        elif func_num == 2:
            fes = 1500
        elif func_num == 3:
            fes = 2000 
        else:
            fes = 2500

        op = DE_optimizer(func_num)
        op.run(fes)
        
        best_input, best_value = op.get_optimal()
        print(best_input, best_value)
        
        with open("{}_function{}.txt".format(__file__.split('_')[0], func_num), 'w+') as f:
            for i in range(op.dim):
                f.write("{}\n".format(best_input[i]))
            f.write("{}\n".format(best_value))
        func_num += 1  
