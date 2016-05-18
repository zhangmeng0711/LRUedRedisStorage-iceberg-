package com.ifeng.iceberg;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Map;
import java.util.Optional;
import java.util.Properties;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 属性加载工具
 *
 */
public final class PropertiesUtils {
    private static final Logger logger = LogManager.getLogger(PropertiesUtils.class);

    private static ConcurrentHashMap<String, String> sysProps = new ConcurrentHashMap<>();

    //禁止实例化
    private PropertiesUtils() {
    }

    static {
        Optional<Map> optional = Optional.of(sysProps);
        optional.ifPresent((Map map) -> {
            map.clear();
        });
        try {
            Path classPath = Paths.get(PropertiesUtils.class.getResource("/").toURI());
            //Path classPath = Paths.get("./");
            System.out.println(classPath.toAbsolutePath());
            Path configPath = classPath.resolve("conf");
            if (Files.exists(configPath)) {
                logger.info("config dir used:" + configPath.toString());
                System.out.println(configPath.toString());
                DirectoryStream<Path> configStream = Files.newDirectoryStream(configPath, (Path path) -> {
                    return path.toString().endsWith(".conf");
                });
                configStream.forEach((Path path) -> {
                    Properties props = new Properties();
                    try (FileInputStream fis = new FileInputStream(path.toString());){
                        props.load(fis);
                        Set<Object> keySet = props.keySet();
                        keySet.forEach((Object key) -> {
                            sysProps.put((String) key, props.getProperty((String) key));
                        });
                    } catch (IOException e) {
                        logger.error(e.getMessage(), e);
                    }
                });
            }
        }catch (Exception e) {
            logger.error("Error filter property files", e);
        }

    }

    public static String getSysProperty(String key) {
        return sysProps.get(key);
    }
}
