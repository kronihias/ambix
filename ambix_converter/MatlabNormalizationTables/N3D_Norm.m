function val = N3D_Norm(acn)
% return N3D weighting factor for ACN (Ambisonic Channel Numbering)
% without factor 1/sqrt(4*pi) !
% acn can be a vector
% 2016 Matthias Kronlachner

% order
n = floor(sqrt(acn));

%degree
m = acn-n.^2-n;

val = sqrt((2.*n+1).*(2-kronDel(m,0)).*factorial(n-abs(m))./factorial(n+abs(m)));
