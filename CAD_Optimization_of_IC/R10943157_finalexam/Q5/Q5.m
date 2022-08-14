close all;
clear all;
clc;

G = [ 3 -1 0 -1  0 0 0 0;
         -1 3 -1 0 0 0 0 0;
         0 -1 4 -1 0 -1 0 0;
         -1 0 -1 4 -1 0 0 0;
         0 0 0 -1 4 -1 0 -1;
         0 0 -1 0 -1 4 -1 0;
         0 0 0 0 0 -1 3 -1;
         0 0 0 0 -1 0 -1 3 ];
Is = [ 1; 0; 0; 1; 1; 0; 0; 1 ];
V = G \ Is;
Vx = V(8)

x = 0:0.1:10;
Vxplot = repelem(Vx,101);
figure(1);
plot(x,Vxplot);
title('Vx Plot at arbitrary time');
xlabel('Time');
ylabel('Vx');
ylim([0 1]);