#!/bin/bash

./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 10 10 0.01 0.008 ../data/qual_um.dta ~../data/submission_10features_10_epochs.dta
#For quick submission test to see if modifications are any good on quiz

./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 30 20 0.01 0.008 ../data/qual_um.dta ~../data/submission_30features_20_epochs.dta
./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 50 20 0.01 0.008 ../data/qual_um.dta ~../data/submission_50features_20_epochs.dta
./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 100 20 0.01 0.008 ../data/qual_um.dta ~../data/submission_100features_20_epochs.dta
./run_matrix_factorization ../data/train.cdta ../data/probe.cdta 100 100 0.01 0.008 ../data/qual_um.dta ~../data/submission_100features_100_epochs.dta