package main;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

public class bin {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		File file = new File("LED.bin");
		InputStream in = null;
		System.out.println("start to read bin:");
		
		try {
			in = new FileInputStream(file);
			int tempbyte;
			while((tempbyte = in.read()) != 0xff){
				System.out.println(tempbyte);
			}
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
