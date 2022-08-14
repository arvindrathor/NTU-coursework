import sys
bug=0
with open(sys.argv[1],'r') as ch:
    line = ch.readline() 
    while ln!='' :
        bug += int(line[-4]) 
        bug += int(line[-3]) 
        bug += int(line[-2])  
        line = ch.readline() 
print("Total {x} bug(s) based on checkforinitialized, p and checkcorrectchange monitors!".format(x=bug))
