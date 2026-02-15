import org.stockfish.StockfishJNI;

public class StockfishTest {
    private StockfishJNI stockfishJNI = new StockfishJNI();

    @org.junit.Test
    public void testStaticEval(){
        System.out.println(stockfishJNI.staticEvalCp());
    }
}
