
insert into tn_gw_stg (stgcd, stgnm, srp)
values ('MSSQL.SLOW_QUERY', '慢查询查询', '
select top 100
  creation_time,
  last_execution_time,
  total_worker_time,
  total_elapsed_time,
  substring(st.text, qs.statement_start_offset / 2 + 1, (
    case statement_end_offset
    when -1 THEN DATALENGTH(st.text)
    else qs.statement_end_offset
    end - qs.statement_start_offset) / 2 + 1
  ) as statement_text
from sys.dm_exec_query_stats as qs
cross apply sys.dm_exec_sql_text(qs.sql_handle) as st
where 1 = 1
and last_execution_time > {{{lastExecutionTime}}}
order by total_worker_time desc;
');

