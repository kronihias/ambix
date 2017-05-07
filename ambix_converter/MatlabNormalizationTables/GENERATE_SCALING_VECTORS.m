% generate scaling vectors in ACN channel order
% 2016 Matthias Kronlachner

N=10; % up to which order should the scaling vector be generated?

acn=0:(N+1)^2-1;

% maxN (Furse-Malham) to N3D
fuma_n3d = N3D_Norm(acn)./maxN_Norm(acn);

% N3D to maxN (Furse-Malham) 
n3d_fuma = maxN_Norm(acn)./N3D_Norm(acn);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% maxN (Furse-Malham) to SN3D
fuma_sn3d = SN3D_Norm(acn)./maxN_Norm(acn);

% SN3D to maxN (Furse-Malham) 
sn3d_fuma = maxN_Norm(acn)./SN3D_Norm(acn);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SN3D to N3D
sn3d_n3d = N3D_Norm(acn)./SN3D_Norm(acn);

% N3D to SN3D
n3d_sn3d = SN3D_Norm(acn)./N3D_Norm(acn);


%% Generate Condon Shortley Phase
cs = cs_phase(acn);
