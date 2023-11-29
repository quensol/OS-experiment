
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for backupque
-- ----------------------------
DROP TABLE IF EXISTS `backupque`;
CREATE TABLE `backupque`  (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `pname` varchar(20) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `pstatus` int(20) NOT NULL,
  `priority` int(20) NOT NULL,
  `runtime` int(20) NOT NULL,
  `memory` int(20) NOT NULL,
  `memorybegin` int(20) NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 736 CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Compact;

SET FOREIGN_KEY_CHECKS = 1;
