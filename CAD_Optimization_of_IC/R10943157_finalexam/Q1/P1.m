clear all;
close all;
dt=0.1;
n=200;

I=[1 0 0; 0 1 0; 0 0 1];
G=[2/3 -1/3 0; -1/3 5/6 -1/2; 0 -1/2 3/2];

C= [2 -1 0; -1 2 0; 0 0 1];

V1=[0; 0; 0];
Is=[1; 0; 1];
for i=1:n
    V2=(((I-(dt*(inv(C))*G))*V1)+(dt*(inv(C))*Is));
    V1=V2; V3(i)=V2(3);
end
tt=linspace(1,n,n);
figure(1)
plot(tt,V3)
title('V-t diagram (Forward Euler)')
xlabel('t (s)')
ylabel('V (Volt)')

I=[1 0 0; 0 1 0; 0 0 1];
G=[2/3 -1/3 0; -1/3 5/6 -1/2; 0 -1/2 3/2];

C= [2 -1 0; -1 2 0; 0 0 1];

V1=[0; 0; 0];
Is=[1; 0; 1];
for i=1:n
      V2=((inv(0.5*G+(1/dt)*C))*(Is-((0.5*G-(1/dt)*C)*V1)));
    V1=V2; V3(i)=V2(3);
end

tt=linspace(1,n,n);
figure(2)
plot(tt,V3)
title('V-t diagram (Trapezoidal Euler)')
xlabel('t (s)')
ylabel('V (Volt)')