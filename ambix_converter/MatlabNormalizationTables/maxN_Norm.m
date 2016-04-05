function val = maxN_Norm(acn)
% return maxN (FUMA) weighting factor for ACN (Ambisonic Channel Numbering)
% acn can be a vector, defined up to 3rd order (acn=15)
% 2016 Matthias Kronlachner

val = zeros(size(acn));

for i=1:length(val)
    % start from SN3D 
    val(i) = SN3D_Norm(acn(i));
    
    % and apply FUMA weights
    switch acn(i)
        case 0 %W
            val(i) = val(i)*1/sqrt(2);
        case 1 %Y
            val(i) = val(i)*1;
        case 2 %Z
            val(i) = val(i)*1;
        case 3 %X
            val(i) = val(i)*1;
        case 4 %V
            val(i) = val(i)*2/sqrt(3);
        case 5 %T
            val(i) = val(i)*2/sqrt(3);
        case 6 %R
            val(i) = val(i)*1;
        case 7 %S
            val(i) = val(i)*2/sqrt(3);
        case 8 %U
            val(i) = val(i)*2/sqrt(3);
        case 9 %Q
            val(i) = val(i)*sqrt(8/5);
        case 10 %O
            val(i) = val(i)*3/sqrt(5);
        case 11 %M
            val(i) = val(i)*sqrt(45/32);
        case 12 %K
            val(i) = val(i)*1;
        case 13 %L
            val(i) = val(i)*sqrt(45/32);
        case 14 %N
            val(i) = val(i)*3/sqrt(5);
        case 15 %P
            val(i) = val(i)*sqrt(8/5);
    end
    % FUMA Scaling is only defined up to order 3
    
end

