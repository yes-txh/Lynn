enum TaskState {
   TASK_WAITING,
   TASK_RUNNING,
   TASK_TERMINATED, 
}

service Scheduler {
   i64 SubmitJob(1:string job_ad),
   bool ReportTaskState(1:i64 job_id, 2:i64 task_id, 3:TaskState state), 
}
