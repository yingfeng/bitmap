# upbit-single-thread

* Generate data
    
      python generate.py size card
 
* Build index

      ./upbit -m build -d data_path -c card -n size -i index_path

* Profit!

      ./upbit -a ub|ucb|inplace -m update|insert|delete|mix --num-queries num_queries -r num_update_queries -c card -i index_path -n size 
