function val = cs_phase(acn)
% return condon shortley phase for the acn channel index
% acn can be a vector
% 2016 Matthias Kronlachner

% order
n = floor(sqrt(acn));

%degree
m = acn-n.^2-n;

val = (-1).^m;

