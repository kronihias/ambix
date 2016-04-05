function d = kronDel(j, k)
% Kronecker Delta
% if j=k returns 1
% otherwise 0
% j can be a vector
% k is a scalar

d = zeros(size(j));

for i=1:length(d);
    
    if j(i) == k
        d(i) = 1;
    else
        d(i) = 0;
    end
    
end