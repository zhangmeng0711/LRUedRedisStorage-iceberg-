package com.ifeng.iceberg;

import com.google.common.collect.Lists;

import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.net.URLEncoder;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.List;
import java.util.Random;

/**
 * <p>Title: </p>
 * <p>Description: calculate hosts and position of the data want to query</p>
 * <p>Company: ifeng.com</p>
 *
 * @author :zhangmeng1
 * @version 1.0
 *          <p>------------------------------------------------------------------------------------------------------------------
 *          <p>date                   author                    action
 *          <p> 2016/3/11              zhangmeng1                  initial
 *          <p>
 *          <p>------------------------------------------------------------------------------------------------------------------
 */
public class IcebergClient {
    private int dir_num;
    private List<String[]> ips = Lists.newArrayList();
    private static IcebergClient client;
    private IcebergClient(){
        String[] masters ;
        String[] slaves;
        masters = PropertiesUtils.getSysProperty("masters").split(";");
        slaves = PropertiesUtils.getSysProperty("slaves").split(";");
        ips.add(masters);
        ips.add(slaves);
        dir_num = Integer.parseInt( PropertiesUtils.getSysProperty("dir_num"));
    }
    public  static IcebergClient getInstance(){
        if(client == null){
            client = new IcebergClient();
        }
        return client;
    }

    /**
     * query host
     * @param key
     * @return
     */
    public String getHost(String key){
        Random random = new Random();
        String[] address = ips.get(random.nextInt(ips.size()));
        int num = hash(key, address.length);
        return address[num];
    }
    public static int hash(String input, int slot) {
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            byte[] messageDigest = md.digest(input.getBytes());
            long  total =0;
            int i;
            for(byte b:messageDigest){
                i = 0x0FF & b;
                total += i;
            }
            BigInteger number = new BigInteger(1, messageDigest);
            String hashtext = number.toString(16);
            // Now we need to zero pad it if you actually want the full 32 chars.
            while (hashtext.length() < 32) {
                hashtext = "0" + hashtext;
            }
            return (int) (total%slot);
        }
        catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * get position in nginx address
     * @param key
     * @return
     * @throws UnsupportedEncodingException
     */
    public String getPath(String key) throws UnsupportedEncodingException {
        int dir1 = hash(key, dir_num);
        String newKey = key + String.format("%03d", dir1);
        int dir2 =  hash(newKey, dir_num);
        newKey += String.format("%03d", dir2);
        int dir3 =  hash(newKey, dir_num);
        return "/" + String.format("%03d", dir1)  + "/" + String.format("%03d", dir2) + "/" + String.format("%03d", dir3) + "/" + URLEncoder.encode(key, "utf-8");
    }

    /**
     * get address of the data
     * @param key
     * @return
     * @throws UnsupportedEncodingException
     */
    public String getPosition(String key) throws UnsupportedEncodingException {
        String res =  "http://" + getHost(key) + ":8090/files" + getPath(key);
        return res;
    }

}
