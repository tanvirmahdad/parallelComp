 
============================================================
=====     Queued job information at submit time        =====
============================================================
  The submitted file is: myscript.sh
  The time limit is 00:30:00 HH:MM:SS.
  The target directory is: /home/uabclsa0020/hw1
  The memory limit is: 1gb
  The job will start running after: 2021-02-08T11:24:54
  Job Name: myscriptshSCRIPT
  Virtual queue: class
  QOS: --qos=class
  Constraints: --constraint=myscriptshdmc
  Command typed:
/apps/scripts/run_script myscript.sh     
  Queue submit command:
sbatch --qos=class -J myscriptshSCRIPT --begin=2021-02-08T11:24:54 --requeue --mail-user=mahdad@uab.edu -o myscriptshSCRIPT.o$SLURM_JOB_ID -t 00:30:00 -N 1-1 -n 1 --mem-per-cpu=1000 --constraint=myscriptshdmc 
  Job number: 
 
