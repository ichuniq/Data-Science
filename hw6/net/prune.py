import numpy as np
import torch
from torch.nn.modules.module import Module


class PruningModule(Module):
    def prune_by_percentile(self, q={'conv1': 16, 'conv2': 62, 'conv3': 65, 'conv4': 63, 'conv5': 63, 'fc1': 91, 'fc2': 91, 'fc3': 75}):
        ########################
        # TODO
        # 	For each layer of weights W (including fc and conv layers) in the model, obtain the qth percentile of W as
        # 	the threshold, and then set the nodes with weight W less than threshold to 0, and the rest remain unchanged.
        ########################

        # Calculate percentile value
        percent_dict = {}
        for name, p in self.named_parameters():
            if 'bias' in name:
                continue
            # name: conv1.weight, fc1.weight, .etc
            tensor = p.data.cpu().numpy()
            W_arr = tensor[np.nonzero(tensor)]  # flattened array of nonzero values
            # [:-7] remove ".weight"
            percent_dict[name[:-7]] = np.percentile(abs(W_arr), q[name[:-7]]) 
            print('{} pruning with threshold = {}'.format(name, percent_dict[name[:-7]]))
        
        # Prune the weights and mask
        for name, module in self.named_modules():
            if name in ['conv1','conv2','conv3','conv4','conv5', 'fc1', 'fc2', 'fc3']:
                self.prune(module=module, threshold=percent_dict[name])
        # pass

    def prune_by_std(self, s=0.25):
        s = 1.7
        for name, module in self.named_modules():

            #################################
            # TODO:
            #    Only fully connected layers were considered, but convolution layers also needed
            #################################
            # pass
            if name in ['conv1', 'conv2', 'conv3', 'conv4', 'conv5', 'fc1', 'fc2', 'fc3']:
                threshold = np.std(module.weight.data.cpu().numpy()) * s
                print(f'Pruning with threshold : {threshold} for {name}')
                self.prune(module, threshold)

    def prune(self, module, threshold):

        #################################
        # TODO:
        #    1. Use "module.weight.data" to get the weights of a certain layer of the model
        #    2. Set weights whose absolute value is less than threshold to 0, and keep the rest unchanged
        #    3. Save the results of the step 2 back to "module.weight.data"
        #    --------------------------------------------------------
        #    In addition, there is no need to return in this function ("module" can be considered as call by
        #    reference)
        #################################
        # Convert Tensors to numpy and calculate
        device = module.weight.device
        tensor = module.weight.data.cpu().numpy()
        mask = np.where(abs(tensor) < threshold, 0, 1)
        # Apply new weight
        module.weight.data = torch.from_numpy(tensor * mask).to(device).float()
        # pass
