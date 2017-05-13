% This script converts the binaural decoder for YouTube to be used in
% ambix binaural - resulting in a preset that matches youtube spatial audio
% playback
%
% Raw material:
%
% https://github.com/google/spatial-media/tree/master/spatial-audio


%
% load the symmetric-ambisonic-binaural-decoder

[bin_dec_0, fs] = audioread('symmetric-ambisonic-binaural-decoder/binaural_decoder_0.wav');
[bin_dec_1, fs] = audioread('symmetric-ambisonic-binaural-decoder/binaural_decoder_1.wav');
[bin_dec_2, fs] = audioread('symmetric-ambisonic-binaural-decoder/binaural_decoder_2.wav');
[bin_dec_3, fs] = audioread('symmetric-ambisonic-binaural-decoder/binaural_decoder_3.wav');

%
%
% create the stereo wav files to be used in ambix_binaural according to
% signal flow on https://github.com/google/spatial-media/tree/master/spatial-audio

stereo_bin_0 = [bin_dec_0 bin_dec_0];
stereo_bin_1 = [bin_dec_1 -bin_dec_1];
stereo_bin_2 = [bin_dec_2 bin_dec_2];
stereo_bin_3 = [bin_dec_3 bin_dec_3];


audiowrite('stereo_bin_dec_0.wav',stereo_bin_0,fs,'BitsPerSample',32);
audiowrite('stereo_bin_dec_1.wav',stereo_bin_1,fs,'BitsPerSample',32);
audiowrite('stereo_bin_dec_2.wav',stereo_bin_2,fs,'BitsPerSample',32);
audiowrite('stereo_bin_dec_3.wav',stereo_bin_3,fs,'BitsPerSample',32);

