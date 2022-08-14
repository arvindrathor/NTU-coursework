syms G C I dt s;
G=[2 -1  0; 
  -1  2 -1; 
   0 -1  2; 
   ];
C =[2  -1  0; 
	-1  2  0; 
	0  0  1 ];
I=[1; 0; 1];
dt=0.1;

Vo=zeros(3,2001);
for t=2:2001
    Vo(:,t)=(eye(3)-dt*(C\G))*Vo(:,t-1)+dt*(C\I);
end
figure(1);
plot(0:0.01:20,Vo(1,:));
title('V-t diagram (Forward Euler)')
xlabel('t (s)')
ylabel('V (Volt)')

V=zeros(3,2001);
for t=2:2001
    V(:,t)=(dt\C+G)\(dt\C*V(:,t-1)+I);
end
figure(2)
plot(0:0.01:20,V(1,:));
title('V-t diagram (Backward Euler)')
xlabel('t (s)')
ylabel('V (Volt)')

syms G C I dt;
G=[2 -1  0; 
  -1  2 -1; 
   0 -1  2; 
   ];
C =[2  -1  0; 
 -1  2  0; 
 0  0  1 ];
I=[1; 0; 1];
dt=0.1;
dt=0.01;

V=zeros(3,2001);
for t=2:2001
    V(:,t)=(dt\C+2\G)\((dt\C-2\G)*V(:,t-1)+I);
end
figure(3)
plot(0:0.01:20,V(2,:));
title('V-t diagram (Trapezoidal Euler)')
xlabel('t (s)')
ylabel('V (Volt)')

I=[1/s;0;0;];
A=G\C;
[W,L]=eig(A);
V=W*((eye(3)+s*L)\(W\(G\I)));
Vt=ilaplace(V);
figure(4)
fplot(real(Vt(2)*2),[0,20]);
title('V-t diagram (Eigenvector method)')
xlabel('t (s)')
ylabel('V (Volt)')

result = LTspice2Matlab('Q2d.raw')
% Vx-t diagram from LTSpice
% V(vx) is at the 2rd entry of result.variable_mat and result.variable_name_list
i = 2;
figure(5)
plot(result.time_vect, result.variable_mat(i,:));
title('Vx-t diagram from LTSpice');
xlabel('Time [s]');
ylab = sprintf('%s [V]',result.variable_name_list{i});
ylabel(ylab)


