function [L, D] = LDLT(A)
[m n] = size(A);
L = zeros(n, n);
D = zeros(n, n);
D(1, 1) = A(1, 1);
L(1, 1) = 1;
% j = 1
for i = 2 : n
    L(i, 1) = A(i, 1) / D(1, 1);
end
% j = 2 : n-1
for j = 2 : n-1
    L(j, j) = 1;
    D(j, j) = A(j, j);
    for k = 1 : j-1
        D(j, j) = D(j, j) - L(j, k)*L(j, k)*D(k, k);
    end
    for i = j+1 : n
        L(i, j) = A(i, j);
        for k = 1 : j-1
            L(i, j) = L(i, j) - L(i, k)*L(j, k)*D(k, k);
        end
        L(i, j) = L(i, j) / D(j, j);
    end
end
% j = n
L(n, n) = 1;
D(n, n) = A(n, n);
for k = 1 : n-1
    D(n, n) = D(n, n) - L(n, k)*L(n, k)*D(k, k);
end
    