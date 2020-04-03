from bs4 import BeautifulSoup
from urllib.request import urlopen
import sys
import re

attr_list = []
with open('./input_hw1.txt', 'r') as f:
    for line in f:
        url_list = []
        url_list.append(line[:-1])
        url = 'https://www.blockchain.com/eth/address/'+ line[:-1] +'?view=standard'
        count = 0
        while (count <= 3):
            html = urlopen(url).read()
            soup = BeautifulSoup(html, "html.parser")
            detail = soup.find('div', "hnfgic-0 blXlQu").find_all('div', "sc-8sty72-0 cyLejs")
            for i in range(2, len(detail), 2):
                attr_list.append(detail[i].text.strip()+': '+detail[i+1].text.strip())
                
            trans = soup.find('div', "sc-1d6wz2a-0 gqRKsm").find_all('div', "sc-1fp9csv-0 gkLWFf")
            for row in trans[::-1]:
                sub_hash = row.find_all('div', "ccso3i-0 dMfwHf")
                to_addr = sub_hash[3].text.strip()
                if (to_addr != url_list[-1].lower()):
                    print(to_addr)
                    date = sub_hash[1].text.strip()
                    amount = sub_fee = row.find_all('div', "ccso3i-0 jSawfE")[0].text.strip()
                    attr_list.append('Date: ' + date)
                    attr_list.append('To: ' + to_addr)
                    attr_list.append('Amount: ' + amount)
                    break
            attr_list.append('--------------------------------------------------------------------------')
            if (to_addr==url_list[-1]):
                break
            url_list.append(to_addr)
            url = 'https://www.blockchain.com/eth/address/'+ to_addr +'?view=standard'
            count+=1
            
        if (count==4): 
            url_list.pop()
        result = ''
        for u in url_list[:-1]:
            result += u + ' -> '
        result += url_list[-1]
        attr_list.append(result)
        attr_list.append('--------------------------------------------------------------------------')