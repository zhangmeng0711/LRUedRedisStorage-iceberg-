package com.ifeng.iceberg;

import java.io.UnsupportedEncodingException;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Company: ifeng.com</p>
 *
 * @author :zhangmeng1
 * @version 1.0
 *          <p>------------------------------------------------------------------------------------------------------------------
 *          <p>date                   author                    action
 *          <p> 2016/5/18              zhangmeng1                  initial
 *          <p>
 *          <p>------------------------------------------------------------------------------------------------------------------
 */
public class Test {
    public static void main(String[] args) throws UnsupportedEncodingException {
        IcebergClient client = IcebergClient.getInstance();
        String key = "358142036563870";
        System.out.println(client.getPosition(key));
    }
}
