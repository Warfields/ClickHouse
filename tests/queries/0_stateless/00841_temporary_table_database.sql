CREATE TEMPORARY TABLE t1_00841 (x UInt8);
INSERT INTO t1_00841 VALUES (1);
SELECT * FROM t1_00841;

CREATE TEMPORARY TABLE test.t2_00841 (x UInt8); -- { serverError 442 }
