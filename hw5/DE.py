import numpy as np
from random import random, uniform
from random import sample
# must use python 3.6, 3.7, 3.8(3.8 not for macOS) for sourcedefender
import sourcedefender
from HomeworkFramework import Function


class RS_optimizer(Function): # need to inherit this class "Function"
    def __init__(self, target_func):
        super().__init__(target_func) # must have this init to work normally

        self.lower = self.f.lower(target_func)
        self.upper = self.f.upper(target_func)
        self.dim = self.f.dimension(target_func)

        self.target_func = target_func

        self.eval_times = 0
        self.optimal_value = float("inf")
        self.optimal_solution = np.empty(self.dim)

    def get_optimal(self):
        return self.optimal_solution, self.optimal_value

    def run(self, FES): # main part for your implementation
        
        while self.eval_times < FES:
            print('=====================FE=====================')
            print(self.eval_times)

            solution = np.random.uniform(np.full(self.dim, self.lower), np.full(self.dim, self.upper), self.dim)
            value = self.f.evaluate(func_num, solution)
            self.eval_times += 1

            if value == "ReachFunctionLimit":
                print("ReachFunctionLimit")
                break            
            if float(value) < self.optimal_value:
                self.optimal_solution[:] = solution
                self.optimal_value = float(value)

            print("optimal: %f\n" % self.get_optimal()[1])
            

class DE_optimizer(Function):
    def __init__(self, target_func, mut=0.8, cr=0.7, pop_size=10):
        super().__init__(target_func)

        self.lower = self.f.lower(target_func)
        self.upper = self.f.upper(target_func)
        self.dim = self.f.dimension(target_func)

        self.target_func = target_func

        self.eval_times = 0
        self.optimal_value = float("inf")
        self.optimal_solution = np.empty(self.dim)
        
        self.mut = mut
        self.cr = cr
        self.pop_size = pop_size

    def get_optimal(self):
        return self.optimal_solution, self.optimal_value

    def run(self, FES):
        # initialize population
        pop = np.asarray([[uniform(self.lower, self.upper) for j in range(self.dim)] for i in range(self.pop_size)])
        # initialize fitness
        fitness = np.asarray([self.f.evaluate(func_num, row) for row in pop])
        self.eval_times += self.pop_size
        best_idx = np.argmin(fitness)
        self.optimal_solution[:] = pop[best_idx]

        while self.eval_times <= FES:
            #print('=====================FE=====================')
            print("=====eval_times: ", self.eval_times)

            for j in range(self.pop_size):
                # Get 3 ranfom idx, exclude j
                idxs = [idx for idx in range(self.pop_size) if idx != j]
                # rand_idxs = sample(idxs, 3)
                # a, b, c = pop[rand_idxs[0]], pop[rand_idxs[1]], pop[rand_idxs[2]]
                #------Mutation------
                target = pop[j]
                a, b, c = pop[np.random.choice(idxs, 3, replace = False)]
                mutant = a + self.mut * (b - c)
                mutant = np.clip(mutant, self.lower, self.upper)

                #------Recombination------
                cross_points = np.random.rand(self.dim) <= self.cr   #array([[False,  True, False,  True],...] dtype=bool)
                if not np.any(cross_points):
                    cross_points[np.random.randint(0, self.dim)] = True
                
                #------Selection------
                trial = np.where(cross_points==True, mutant, target)
                score_trail = self.f.evaluate(func_num, trial)
                self.eval_times += 1
                if score_trail == "ReachFunctionLimit":
                    print("ReachFunctionLimit!!!")
                    break

                if (score_trail < fitness[j]):
                    fitness[j] = score_trail
                    pop[j] = trial
                    if (score_trail < fitness[best_idx]):
                        best_idx = j
                        self.optimal_solution[:] = trial
                        # for _ in range(self.dim):
                        #     print("{}\n".format(trial))

            self.optimal_value =  fitness[best_idx]
            print("optimal: {}".format(self.get_optimal()[1]))




if __name__ == '__main__':
    func_num = 1
    fes = 0 # number constraint for calling function
    #function1: 1000, function2: 1500, function3: 2000, function4: 2500
    while func_num < 5:
        if func_num == 1:
            fes = 1000
        elif func_num == 2:
            fes = 1500
        elif func_num == 3:
            fes = 2000 
        else:
            fes = 2500

        # you should implement your optimizer
        # op = RS_optimizer(func_num)
        op = DE_optimizer(func_num)
        op.run(fes)
        
        best_input, best_value = op.get_optimal()
        print(best_input, best_value)
        
        # change the name of this file to your student_ID and it will output properlly
        with open("{}_function{}.txt".format(__file__.split('_')[0], func_num), 'w+') as f:
            for i in range(op.dim):
                f.write("{}\n".format(best_input[i]))
            f.write("{}\n".format(best_value))
        func_num += 1  
