#!/bin/bash

#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 20 20 0.01 0.008 20 ../neighborhood/knn20.txt ../data/qual.cdta ../submissions/probe_20features_20epochs.dta ../submissions/qual_20features_20epochs.dta

./run_matrix_factorization2* ../data/train.cdta ../data/probe.cdta 30 20 0.01 0.008 20 ../neighborhood/knn20.txt ../data/qual.cdta ../submissions/probe_30features_20epochs_20neighbors.dta ../submissions/qual_30features_20epochs_20neighbors.dta

#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 50 20 0.01 0.008 50 ../neighborhood/knn50.txt ../data/qual.cdta ../submissions/probe_50features_20epochs.dta ../submissions/qual_50features_20epochs.dta
#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 100 20 0.01 0.008 50 ../neighborhood/knn50.txt ../data/qual.cdta ../submissions/probe_100features_20epochs.dta ../submissions/qual_100features_20epochs.dta
#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 100 100 0.01 0.008 50 ../neighborhood/knn50.txt ../data/qual.cdta ../submissions/probe_100features_100epochs.dta ../submissions/qual_100features_100epochs.dta

#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 10 10 0.01 0.008 50 ../neighborhood/knn50.txt ../data/qual.cdta ../submissions/probe_10features_10epochs.dta ../submissions/qual_10features_10epochs.dta
#For quick submission test to see if modifications are any good on quiz

#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 30 20 0.01 0.008 ../data/qual_um.dta ../data/submission_30features_20epochs.dta
#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 50 20 0.01 0.008 ../data/qual_um.dta ../data/submission_50features_20epochs.dta
#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 100 20 0.01 0.008 ../data/qual_um.dta ../data/submission_100features_20epochs.dta
#./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 100 100 0.01 0.008 ../data/qual_um.dta ../data/submission_100features_100epochs.dta
