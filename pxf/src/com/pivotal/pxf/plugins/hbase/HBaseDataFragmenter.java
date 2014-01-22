package com.pivotal.pxf.plugins.hbase;

import com.pivotal.pxf.api.Fragment;
import com.pivotal.pxf.api.Fragmenter;
import com.pivotal.pxf.api.utilities.InputData;
import com.pivotal.pxf.plugins.hbase.utilities.HBaseLookupTable;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.HRegionInfo;
import org.apache.hadoop.hbase.ServerName;
import org.apache.hadoop.hbase.client.HTable;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;

/*
 * Fragmenter class for HBase data resources.
 *
 * Extends the Fragmenter abstract class, with the purpose of transforming
 * an input data path (an HBase table name in this case) into a list of regions
 * that belong to this table.
 *
 * Fragmenter also puts HBase lookup table information for the given
 * table (if exists) in fragments' user data field.
 */
public class HBaseDataFragmenter extends Fragmenter {

    public HBaseDataFragmenter(InputData inConf) {
        super(inConf);
    }

    @Override
    public List<Fragment> getFragments() throws Exception {
        byte[] userData = prepareUserData();
        addTableFragments(userData);

        return fragments;
    }

    private byte[] prepareUserData() throws IOException {
        HBaseLookupTable lookupTable = new HBaseLookupTable();
        Map<String, byte[]> mappings = lookupTable.getMappings(inputData.tableName());
        lookupTable.close();

        if (mappings != null) {
            return serializeMap(mappings);
        }

        return null;
    }

    private byte[] prepareFragmentMetadata(HRegionInfo region) throws IOException {

        ByteArrayOutputStream byteArrayStream = new ByteArrayOutputStream();
        ObjectOutputStream objectStream = new ObjectOutputStream(byteArrayStream);
        objectStream.writeObject(region.getStartKey());
        objectStream.writeObject(region.getEndKey());

        return byteArrayStream.toByteArray();
    }

    private void addTableFragments(byte[] userData) throws IOException {
        HTable table = new HTable(HBaseConfiguration.create(), inputData.tableName());
        NavigableMap<HRegionInfo, ServerName> locations = table.getRegionLocations();

        for (Map.Entry<HRegionInfo, ServerName> entry : locations.entrySet()) {
            addFragment(entry, userData);
        }

        table.close();
    }

    private void addFragment(Map.Entry<HRegionInfo, ServerName> entry,
                             byte[] userData) throws IOException {
        ServerName serverInfo = entry.getValue();
        String[] hosts = new String[]{serverInfo.getHostname()};
        HRegionInfo region = entry.getKey();
        byte[] fragmentMetadata = prepareFragmentMetadata(region);
        Fragment fragment = new Fragment(inputData.tableName(), hosts, fragmentMetadata, userData);
        fragments.add(fragment);
    }

    private byte[] serializeMap(Map<String, byte[]> tableMappings) throws IOException {
        ByteArrayOutputStream byteArrayStream = new ByteArrayOutputStream();
        ObjectOutputStream objectStream = new ObjectOutputStream(byteArrayStream);
        objectStream.writeObject(tableMappings);

        return byteArrayStream.toByteArray();
    }
}